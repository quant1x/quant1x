#pragma once
#ifndef QUANT1X_NETWORK_IO_CONNECTION_POOL_H
#define QUANT1X_NETWORK_IO_CONNECTION_POOL_H 1

#include <quant1x/io/endpoint.h>
#include <quant1x/io/operation_handler.h>
#include <quant1x/runtime/core.h>
#include <quant1x/std/except.h>
#include <quant1x/std/util.h>

#include <queue>
#include <shared_mutex>
#include <utility>

/**
 * @brief RAII管理的TCP连接包装类
 * @note 生命周期完全由TcpConnectionPool控制
 * @warning 禁止拷贝和移动构造，仅能通过unique
 */
class Connection {
public:
    /**
     * @brief 构造函数(接管已连接的socket)
     * @param socket 已建立的TCP socket连接
     * @param endpoint 该连接对应的远程端点
     */
    Connection(asio::ip::tcp::socket socket, asio::ip::tcp::endpoint endpoint)
        : socket_(std::move(socket)), endpoint_(std::move(endpoint)) {
        if (!socket_.is_open()) {
            throw std::invalid_argument("Socket must be connected");
        }
    }

    /**
     * @brief 析构函数 - 自动关闭socket连接
     * @note 不会释放Endpoint资源(由连接池管理)
     */
    ~Connection() { close(); }

    // 禁用拷贝和移动
    Connection(const Connection &)            = delete;
    Connection &operator=(const Connection &) = delete;
    Connection(Connection &&)                 = delete;
    Connection &operator=(Connection &&)      = delete;

    /**
     * @brief 获取底层socket引用(非线程安全)
     * @return 可读写的socket引用
     */
    asio::ip::tcp::socket &socket() noexcept { return socket_; }

    /**
     * @brief 获取连接的远程端点
     * @return 端点常量引用
     */
    [[nodiscard]] const asio::ip::tcp::endpoint &endpoint() const noexcept { return endpoint_; }

    /**
     * @brief 显式关闭连接
     * @note 可重复调用，线程不安全
     */
    void close() {
        if (isOpen()) {
            asio::error_code ec;
            socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
            socket_.close(ec);
            // 不抛出异常，允许在析构函数中调用
        }
    }

    /**
     * @brief 检查连接是否仍然打开
     */
    [[nodiscard]] bool isOpen() const noexcept { return socket_.is_open(); }

private:
    asio::ip::tcp::socket         socket_;    ///< 底层TCP socket
    const asio::ip::tcp::endpoint endpoint_;  ///< 连接的远程端点
};

/**
 * @brief TCP连接池实现，管理多个endpoint的连接
 */
template <typename Handler>
class TcpConnectionPool {
public:
    /**
     * @brief 构造函数
     * @param min_connections 最小连接数
     * @param max_connections 最大连接数
     * @param network_handler 网络操作处理器
     */
    TcpConnectionPool(size_t min_connections, size_t max_connections, std::shared_ptr<Handler> network_handler)
        : min_connections_(min_connections)
        , max_connections_(max_connections)
        , endpoint_weight_(1)
        , network_handler_(std::move(network_handler))
        , endpoint_manager_(std::make_shared<EndpointManager>())
        , io_context_(std::make_shared<asio::io_context>())
        , work_guard_(asio::make_work_guard(*io_context_)) {
        // 参数校验
        if (min_connections_ > max_connections_) {
            throw std::invalid_argument("min_connections cannot be greater than max_connections");
        }
        if (max_connections_ == 0) {
            throw std::invalid_argument("max_connections cannot be zero");
        }

        if (!network_handler_) {
            throw std::invalid_argument("network_handler cannot be null");
        }

        spdlog::debug("[connection pool] min_connections={}, max_connections={}, endpoint_weight={}",
                      min_connections_,
                      max_connections_,
                      endpoint_weight_);

        // 创建IO上下文线程池
        int num_threads = 2;  // 使用 2 个线程
        for (int i = 0; i < num_threads; ++i) {
            io_threads_.emplace_back([this, i]() {
                // 自动绑定到最优CPU
                std::error_code ec{};
                if (affinity::bind_current_thread_to_optimal_cpu(ec)) {
                    spdlog::debug("thread[{}] affinity, ok", i);
                } else {
                    // 亲和性的错误是可以忽略的
                    spdlog::warn("Thread {} affinity with exception: [{}:{}]", i, ec.value(), ec.message());
                }
                u64 thread_id = util::get_thread_id(std::this_thread::get_id());
                spdlog::debug("Thread {} started (ID: {})", i, thread_id);

                try {
                    io_context_->run();  // 运行事件循环
                    spdlog::info("Thread {} finished normally (ID: {})", i, thread_id);
                } catch (const std::exception &e) {
                    spdlog::error("Thread {} exited with exception: {} (ID: {})", i, e.what(), thread_id);
                }
            });
            // 确保线程已启动(可选)
            // std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        start();
    }

    /**
     * @brief 析构函数，自动停止连接池
     */
    ~TcpConnectionPool() {
        spdlog::warn("正在关闭连接池...");
        stop();
        // endpoint_manager_.reset();
        // network_handler_.reset();
        // io_context_.reset();
        spdlog::warn("正在关闭线程池...");
        try {
            // 等待所有线程完成
            for (auto &t : io_threads_) {
                if (!t.joinable()) {
                    spdlog::warn("发现不可 join 的线程对象");
                    continue;
                }
                if (t.get_id() == std::thread::id()) {
                    spdlog::warn("发现无效线程对象");
                    continue;
                }
                u64 tid = util::get_thread_id(t.get_id());
                spdlog::warn("关闭线程{}", tid);
                if (t.joinable()) {
                    t.join();
                }
                spdlog::warn("关闭线程{}, finished.", tid);
            }
        } catch (const std::exception &e) {  // 其他标准异常
            spdlog::error("[connection_pool] - 标准异常: {} (type: {})", e.what(), typeid(e).name());
            // 对于system_error可以记录更多信息
            if (auto se = dynamic_cast<const std::system_error *>(&e)) {
                spdlog::error("Error code: {}, category: {}", se->code().value(), se->code().category().name());
            }
        } catch (...) {
            spdlog::error("获取日K线异常");
        }
        spdlog::warn("正在关闭线程池...OK");
        spdlog::warn("正在关闭连接池...OK");
    }

    /**
     * @brief 添加一个端点到端点管理器
     *
     * @param ip_address 端点的IP地址
     * @param port 端点的端口号
     * @param weight 端点的权重值，默认为0表示使用默认权重
     * @return bool 添加成功返回true，失败返回false
     */
    bool add_endpoint(const std::string &ip_address, unsigned short port, size_t weight = 0) {
        return endpoint_manager_->addEndpoint(ip_address, port, weight == 0 ? endpoint_weight_ : weight);
    }

    /**
     * @brief 添加一个TCP端点
     *
     * @param endpoint 要添加的TCP端点
     * @param weight 端点的权重值，默认为0表示使用默认权重
     * @return bool 添加成功返回true，失败返回false
     */
    bool add_endpoint(const asio::ip::tcp::endpoint &endpoint, size_t weight = 0) {
        return endpoint_manager_->addEndpoint(endpoint, weight == 0 ? endpoint_weight_ : weight);
    }

    /**
     * @brief 从连接池获取一个可用连接或创建新连接
     *
     * 该方法首先尝试从空闲连接池获取可用连接，如果没有可用连接则创建新连接。
     * 返回的连接对象使用自定义删除器，当连接不再使用时自动归还到连接池。
     *
     * @return std::unique_ptr<Connection, std::function<void(Connection*)>> 带有自动归还功能的连接指针
     * @throws std::runtime_error 当没有可用端点或连接超时时抛出
     * @throws std::system_error 当系统级连接错误发生时抛出
     * @throws std::exception 当握手失败或其他连接错误发生时抛出
     *
     * @note 返回的连接对象会在析构时自动调用release()方法归还到连接池
     * @warning 调用者必须确保在连接使用期间io_context和endpoint_manager保持有效
     */
    std::unique_ptr<Connection, std::function<void(Connection *)>> acquire() {
        // 1. 首先尝试从空闲连接池获取
        std::unique_ptr<Connection> raw_conn;
        {
            std::lock_guard<std::mutex> lock(connections_mutex_);
            if (!idle_connections_.empty()) {
                raw_conn = std::move(idle_connections_.front());
                idle_connections_.pop_front();
                idle_connection_count_--;
                spdlog::debug("Reused connection from pool (ID: {})", reinterpret_cast<uintptr_t>(raw_conn.get()));
            }
        }

        // 2. 如果没有可用连接，创建新连接
        asio::ip::tcp::endpoint endpoint;
        if (!raw_conn) {
            spdlog::debug("Creating new connection...");

            // 2.1 获取可用endpoint
            auto endpoint_opt = endpoint_manager_->acquireEndpoint();
            if (!endpoint_opt) {
                spdlog::error("No available endpoints");
                throw std::runtime_error("No available endpoints");
            }
            endpoint = *endpoint_opt;

            // 2.2 创建并连接socket
            asio::ip::tcp::socket socket(*io_context_);
            try {
                // 设置socket选项
                socket.open(asio::ip::tcp::v4());
                socket.set_option(asio::socket_base::reuse_address(true));
                socket.set_option(asio::ip::tcp::no_delay(true));  // 禁用Nagle算法
                socket.bind(asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 0));

                // 带超时的异步连接
                std::future<void> connect_future = socket.async_connect(endpoint, asio::use_future);

                if (connect_future.wait_for(network_handler_->timeout()) == std::future_status::timeout) {
                    socket.close();
                    endpoint_manager_->releaseEndpoint(endpoint);
                    spdlog::error("Connection timeout to {}:{}", endpoint.address().to_string(), endpoint.port());
                    throw std::runtime_error("Connection timeout");
                }
                connect_future.get();  // 检查连接错误

                // 2.3 执行握手协议
                if (!network_handler_->handshake(socket)) {
                    socket.close();
                    endpoint_manager_->releaseEndpoint(endpoint);
                    spdlog::error("Handshake failed with {}:{}", endpoint.address().to_string(), endpoint.port());
                    throw std::runtime_error("Handshake failed");
                }

                // 2.4 创建连接对象
                raw_conn = std::make_unique<Connection>(std::move(socket), endpoint);
                spdlog::debug("Created new connection (ID: {})", reinterpret_cast<uintptr_t>(raw_conn.get()));
            } catch (const std::system_error &e) {
                if (socket.is_open())
                    socket.close();
                endpoint_manager_->releaseEndpoint(endpoint);
                spdlog::error("System error while connecting: {} (code: {})", e.what(), e.code().value());
                throw;
            } catch (const std::exception &e) {
                if (socket.is_open())
                    socket.close();
                endpoint_manager_->releaseEndpoint(endpoint);
                spdlog::error("Error while connecting: {}", e.what());
                throw;
            }
        }

        // 3. 创建自动归还包装器
        auto deleter = [this](Connection *conn) {
            if (conn) {
                try {
                    spdlog::debug("Auto-release connection (ID: {})", reinterpret_cast<uintptr_t>(conn));
                    // 复用returnConnection实现
                    this->release(std::unique_ptr<Connection>(conn));
                    spdlog::debug("Auto-released connection (ID: {})", reinterpret_cast<uintptr_t>(conn));
                } catch (const std::exception &e) {
                    spdlog::error("Error during auto-release: {}", e.what());
                }
            }
        };
        active_connection_count_++;
        // 4. 返回带有自定义删除器的unique_ptr
        return std::unique_ptr<Connection, decltype(deleter)>(raw_conn.release(), deleter);
    }

    /**
     * @brief 释放并归还连接对象到连接池
     *
     * 该函数负责将使用完毕的连接对象归还到空闲连接池中，并更新相关计数器。
     * 归还前会先将连接对应的Endpoint归还给Endpoint管理器。
     *
     * @param conn 要释放的Connection对象，以unique_ptr形式传递所有权
     *
     * @note 该函数是线程安全的，内部会获取连接池的互斥锁
     * @note 如果传入空指针，函数会直接返回而不执行任何操作
     *
     * @throws 无显式抛出异常，但Endpoint管理器的releaseEndpoint可能抛出异常
     */
    void release(std::unique_ptr<Connection> conn) {
        if (!conn) {
            return;
        }

        const auto conn_id = reinterpret_cast<uintptr_t>(conn.get());
        spdlog::debug("Returning connection (ID: {})", conn_id);

        // NOTE: Do NOT release the Endpoint when returning the connection to
        // the idle pool. The Endpoint allocation must remain reserved for
        // this connection while it is idle. Releasing the endpoint here can
        // cause the same fastest endpoint to be allocated to new connections
        // while existing idle connections still hold sockets to it, which
        // violates the endpoint's max-connections semantics.
        // The endpoint will be released when the connection is actually
        // closed (see closeConnection) or when an error/exception occurs.

        //        // 2. 检查连接健康状态
        //        bool is_healthy = false;
        //        try {
        //            is_healthy = network_handler_->keepalive(conn->socket());
        //        } catch (const std::exception& e) {
        //            spdlog::warn("Health check failed for connection {}: {}", conn_id, e.what());
        //        }

        std::lock_guard<std::mutex> lock(connections_mutex_);
        idle_connections_.push_back(std::move(conn));
        idle_connection_count_++;
        active_connection_count_--;
        spdlog::debug("Connection {} returned to pool", conn_id);
    }

    /**
     * @brief 关闭并释放空闲连接
     *
     * 该函数负责安全地关闭一个空闲连接，并执行以下操作:
     * 1. 将连接对应的Endpoint归还给管理器
     * 2. 关闭连接本身
     * 3. 减少空闲连接计数器
     *
     * @param idle_conn 要关闭的空闲连接指针，使用unique_ptr管理
     * @note 如果传入的连接指针为空，函数将直接返回而不执行任何操作
     */
    void closeConnection(const std::unique_ptr<Connection> &idle_conn) {
        if (!idle_conn) {
            return;
        }

        const auto conn_id = reinterpret_cast<uintptr_t>(idle_conn.get());
        spdlog::debug("Closing connection (ID: {})", conn_id);

        // 1. 归还Endpoint到管理器
        endpoint_manager_->releaseEndpoint(idle_conn->endpoint());
        // 2. 关闭连接
        idle_conn->close();
        spdlog::debug("Connection {} closed", conn_id);
        // 3. 计数器减一
        idle_connection_count_--;
    }

    /**
     * @brief 启动连接池，开始心跳检测
     */
    void start() {
        if (running_) {
            return;
        }
        running_ = true;
        startHeartbeatTimer();
    }

    /**
     * @brief 停止服务运行
     *
     * 该方法会停止所有活动连接并终止IO上下文的事件循环。
     * 首先设置运行标志为false，然后取消心跳定时器，
     * 关闭所有现有连接，最后停止IO上下文。
     *
     * @note 该方法会确保所有挂起的异步操作被正确处理
     * @post 调用后所有网络活动将停止，IO上下文不再处理事件
     */
    void stop() {
        running_ = false;

        if (heartbeat_timer_) {
            heartbeat_timer_->cancel();
        }

        closeAllConnections();

        asio::post(*io_context_, [this]() { asio::dispatch(*io_context_, [this]() { io_context_->poll(); }); });

        io_context_->stop();
    }

    [[nodiscard]] std::pair<size_t, size_t> getEndpointStats(const std::string &host, unsigned short port) const {
        try {
            asio::ip::address       addr = asio::ip::make_address(host);
            asio::ip::tcp::endpoint ep(addr, port);
            return endpoint_manager_->getEndpointStats(ep);
        } catch (const std::exception &e) {
            throw std::runtime_error(std::string("Invalid endpoint: ") + e.what());
        }
    }

private:
    std::vector<std::thread>                io_threads_;      ///< IO上下文运行线程
    std::atomic<bool>                       running_{false};  ///< 连接池运行状态
    std::mutex                              connections_mutex_;
    std::deque<std::unique_ptr<Connection>> idle_connections_;            ///< 空闲连接
    size_t                                  idle_connection_count_{0};    ///< 空闲连接数量(动态统计)
    size_t                                  active_connection_count_{0};  ///< 活跃连接数量(动态统计)

    const size_t                                               min_connections_;   ///< 最小连接数
    const size_t                                               max_connections_;   ///< 最大连接数
    const size_t                                               endpoint_weight_;   ///< endpoint权重
    std::shared_ptr<Handler>                                   network_handler_;   ///< 网络操作处理器
    std::shared_ptr<EndpointManager>                           endpoint_manager_;  ///< endpoint管理器
    std::shared_ptr<asio::io_context>                          io_context_;        ///< IO上下文
    asio::executor_work_guard<asio::io_context::executor_type> work_guard_;        ///< 保持IO上下文运行
    std::unique_ptr<asio::steady_timer>                        heartbeat_timer_;   ///< 心跳定时器

    /**
     * @brief 启动心跳定时器，定期检查连接状态
     *
     * 创建一个定时器，在指定的检查间隔后触发回调函数。回调函数会执行以下操作:
     * 1. 检查当前是否有错误或服务是否已停止运行
     * 2. 检查所有连接状态
     * 3. 递归调用自身以维持定时循环
     * 4. 尝试创建新的连接
     *
     * @note 定时器会在每次触发后自动重新启动，形成循环检查机制
     * @warning 如果定时器回调中发生错误或服务停止，定时器将自动退出
     */
    void startHeartbeatTimer() {
        heartbeat_timer_ = std::make_unique<asio::steady_timer>(*io_context_);
        heartbeat_timer_->expires_after(network_handler_->check_interval());
        heartbeat_timer_->async_wait([this](asio::error_code ec) {
            if (ec || !running_) {
                spdlog::warn("心跳定时器退出");
                return;
            }
            checkConnections();
            startHeartbeatTimer();
            try_create_connections();
        });
    }

    /**
     * @brief 检查并清理空闲连接池中的无效连接
     *
     * 遍历空闲连接池中的所有连接，通过keepalive检测连接有效性。
     * 自动清理空指针和失效的连接，对异常连接执行关闭操作。
     *
     * @note 此函数是线程安全的，内部使用互斥锁保护连接池操作
     * @throws 无显式抛出异常，但会捕获并处理network_handler_->keepalive()可能抛出的异常
     */
    void checkConnections() {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        idle_connections_.erase(std::remove_if(idle_connections_.begin(),
                                               idle_connections_.end(),
                                               [this](const std::unique_ptr<Connection> &conn) {
                                                   if (!conn) {
                                                       return true;  // 自动清理空指针
                                                   }
                                                   try {
                                                       return !network_handler_->keepalive(conn->socket());
                                                   } catch (...) {
                                                       // conn->close();
                                                       closeConnection(conn);
                                                       return true;  // 异常视为连接失效
                                                   }
                                               }),
                                idle_connections_.end());
    }

    /**
     * @brief 尝试创建新的连接以维持最小连接数
     *
     * 当活跃连接数加上空闲连接数小于最小连接数要求时，会尝试创建新的连接。
     * 最多重试10次，每次重试间隔100毫秒。
     *
     * @note 如果endpoint资源不足或获取连接时发生异常，会提前终止创建过程
     * @throws std::exception 当获取新连接失败时抛出异常
     */
    void try_create_connections() {
        // 如果活跃的连接数不足最小连接数, 则主动创建
        int max_retries = 10;  // 最大重试次数
        int retry_count = 0;
        while (active_connection_count_ + idle_connection_count_ < min_connections_ && retry_count < max_retries) {
            size_t available = endpoint_manager_->getAvailableResources();
            if (available == 0) {
                spdlog::warn("endpoint资源不足, 无法创建新的连接, (retry {}/{})", retry_count, max_retries);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 等待一段时间后重试
                ++retry_count;
                continue;
            } else {
                try {
                    auto conn = acquire();
                    ++retry_count;  // 成功获取连接后重置重试计数
                    spdlog::debug("补充1个连接, endpoint={}", conn->endpoint());
                } catch (const std::exception &e) {
                    spdlog::error("Error acquiring new connection: {}", e.what());
                    break;  // 捕获异常后退出循环
                }
            }
        }
    }

    /**
     * @brief 关闭所有空闲连接并清空连接池
     *
     * 该函数会获取连接池的互斥锁，遍历所有空闲连接并逐个关闭，
     * 然后清空空闲连接列表并将计数器归零。
     *
     * @note 此函数是线程安全的，通过互斥锁保护连接池状态
     */
    void closeAllConnections() {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        for (auto &conn : idle_connections_) {
            if (conn && conn->isOpen()) {
                conn->close();
            }
        }
        idle_connections_.clear();
        idle_connection_count_ = 0;
    }
};

#endif  // QUANT1X_NETWORK_IO_CONNECTION_POOL_H
