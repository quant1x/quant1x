package pool

import (
	"errors"
	"fmt"
	"sync"
	"time"

	"gitee.com/quant1x/gox/logger"
)

// Config 连接池相关配置
type Config struct {
	//连接池中拥有的最小连接数
	InitialCap int
	//最大并发存活连接数
	MaxCap int
	//最大空闲连接
	MaxIdle int
	//生成连接的方法
	Factory func() (any, error)
	//关闭连接的方法
	Close func(any) error
	//检查连接是否有效的方法
	Ping func(any) error
	//连接最大空闲时间，超过该事件则将失效
	IdleTimeout time.Duration
}

type connReq struct {
	idleConn *idleConn
}

// channelPool 存放连接信息
type channelPool struct {
	mu           sync.RWMutex        // 读写锁
	conns        chan *idleConn      // 空闲连接
	idleTimeout  time.Duration       // 空闲时间
	waitTimeOut  time.Duration       // 等待时间
	maxActive    int                 // 最大活跃数
	openingConns int                 // 打开连接数
	connReqs     []chan connReq      // 池满后请求新连接的队列
	factory      func() (any, error) // 新连接工厂
	close        func(any) error     // 关闭
	ping         func(any) error     // ping
}

type idleConn struct {
	conn any
	t    time.Time
}

// NewChannelPool 初始化连接
func NewChannelPool(poolConfig *Config) (Pool, error) {
	if !(poolConfig.InitialCap <= poolConfig.MaxIdle && poolConfig.MaxCap >= poolConfig.MaxIdle && poolConfig.InitialCap >= 0) {
		return nil, errors.New("invalid capacity settings")
	}
	logger.Warnf("init connect pool: MaxCap=%d, InitialCap=%d, MaxIdle=%d", poolConfig.MaxCap, poolConfig.InitialCap, poolConfig.MaxIdle)
	if poolConfig.Factory == nil {
		return nil, errors.New("invalid factory func settings")
	}
	if poolConfig.Close == nil {
		return nil, errors.New("invalid close func settings")
	}

	c := &channelPool{
		conns:        make(chan *idleConn, poolConfig.MaxIdle),
		factory:      poolConfig.Factory,
		close:        poolConfig.Close,
		idleTimeout:  poolConfig.IdleTimeout,
		maxActive:    poolConfig.MaxCap,
		openingConns: poolConfig.InitialCap,
	}

	if poolConfig.Ping != nil {
		c.ping = poolConfig.Ping
	}

	for i := 0; i < poolConfig.InitialCap; i++ {
		conn, err := c.factory()
		if err != nil {
			c.Release()
			return nil, fmt.Errorf("factory is not able to fill the pool: %s", err)
		}
		c.conns <- &idleConn{conn: conn, t: time.Now()}
	}

	return c, nil
}

// getConns 获取所有连接
func (c *channelPool) getConns() chan *idleConn {
	c.mu.Lock()
	conns := c.conns
	c.mu.Unlock()
	return conns
}

// Get 从pool中取一个连接
func (c *channelPool) Get() (any, error) {
	conns := c.getConns()
	if conns == nil {
		return nil, ErrClosed
	}
	for {
		select {
		case wrapConn := <-conns:
			if wrapConn == nil {
				return nil, ErrClosed
			}
			//判断是否超时，超时则丢弃
			if timeout := c.idleTimeout; timeout > 0 {
				if wrapConn.t.Add(timeout).Before(time.Now()) {
					logger.Warnf("空闲超时, 关闭连接.")
					//丢弃并关闭该连接
					_ = c.Close(wrapConn.conn)
					continue
				}
			}
			//判断是否失效，失效则丢弃，如果用户没有设定 ping 方法，就不检查
			if c.ping != nil {
				if err := c.Ping(wrapConn.conn); err != nil {
					logger.Warnf("ping失败, 关闭连接.")
					_ = c.Close(wrapConn.conn)
					continue
				}
			}
			return wrapConn.conn, nil
		default:
			c.mu.Lock()
			//logger.Warnf("default-1")
			if c.openingConns >= c.maxActive {
				//logger.Warnf("default-1: 1")
				req := make(chan connReq, 1)
				c.connReqs = append(c.connReqs, req)
				c.mu.Unlock()
				ret, ok := <-req
				if !ok {
					//logger.Warnf("default-1: 1-1")
					return nil, ErrMaxActiveConnReached
				}
				//logger.Warnf("default-1: 2")
				if timeout := c.idleTimeout; timeout > 0 {
					//logger.Warnf("default-1: 2-1")
					if ret.idleConn.t.Add(timeout).Before(time.Now()) {
						//丢弃并关闭该连接
						//logger.Warnf("default-1: 2-1-1")
						logger.Warnf("超时, 关闭连接.")
						_ = c.Close(ret.idleConn.conn)
						continue
					}
					//logger.Warnf("default-1: 2-2")
				}
				//logger.Warnf("default-1: 3")
				return ret.idleConn.conn, nil
			}
			//logger.Warnf("default-2")
			if c.factory == nil {
				//logger.Warnf("default-2: 1")
				c.mu.Unlock()
				return nil, ErrClosed
			}
			//logger.Warnf("default-3")
			conn, err := c.factory()
			if err != nil {
				//logger.Warnf("default-3: 1")
				c.mu.Unlock()
				return nil, err
			}
			//logger.Warnf("default-4")
			c.openingConns++
			c.mu.Unlock()
			return conn, nil
		}
	}
}

// Put 将连接放回pool中
func (c *channelPool) Put(conn any) error {
	if conn == nil {
		return ErrIsNil
	}

	c.mu.Lock()

	if c.conns == nil {
		c.mu.Unlock()
		logger.Warnf("队列无效, 关闭连接.")
		return c.Close(conn)
	}

	if l := len(c.connReqs); l > 0 {
		req := c.connReqs[0]
		copy(c.connReqs, c.connReqs[1:])
		c.connReqs = c.connReqs[:l-1]
		req <- connReq{
			idleConn: &idleConn{conn: conn, t: time.Now()},
		}
		c.mu.Unlock()
		return nil
	} else {
		select {
		case c.conns <- &idleConn{conn: conn, t: time.Now()}:
			c.mu.Unlock()
			return nil
		default:
			c.mu.Unlock()
			//连接池已满，直接关闭该连接
			logger.Warnf("返还连接, 连接池已满, 关闭连接.")
			return c.Close(conn)
		}
	}
}

// Close 关闭单条连接
func (c *channelPool) Close(conn any) error {
	if conn == nil {
		return ErrIsNil
	}
	c.mu.Lock()
	defer c.mu.Unlock()
	if c.close == nil {
		return nil
	}
	c.openingConns--
	return c.close(conn)
}

// Ping 检查单条连接是否有效
func (c *channelPool) Ping(conn any) error {
	if conn == nil {
		return ErrIsNil
	}
	return c.ping(conn)
}

// Release 释放连接池中所有连接
func (c *channelPool) Release() {
	c.mu.Lock()
	conns := c.conns
	c.conns = nil
	c.factory = nil
	c.ping = nil
	closeFun := c.close
	c.close = nil
	c.mu.Unlock()

	if conns == nil {
		return
	}

	close(conns)
	for wrapConn := range conns {
		_ = closeFun(wrapConn.conn)
	}
}

// CloseAll 仅关闭连接池中所有连接
func (c *channelPool) CloseAll() {
	c.mu.Lock()
	defer c.mu.Unlock()
	if c.conns == nil || c.close == nil {
		return
	}
	select {
	case wrapConn := <-c.conns:
		_ = c.close(wrapConn.conn)
		c.openingConns--
	default:
		break
	}
}

// Len 连接池中已有的连接
func (c *channelPool) Len() int {
	return len(c.getConns())
}
