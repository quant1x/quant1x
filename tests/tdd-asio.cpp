#include "test/test.h"
#include <iostream>

#include <q1x/net/endpoint.h>

TEST_CASE("endpoint pool", "[net]") {
    EndpointManager manager;

    // 添加 endpoint 配置
    asio::ip::tcp::endpoint ep1(asio::ip::address::from_string("127.0.0.1"), 8080);
    manager.addEndpoint(ep1, 2); // 最大2个连接

    asio::ip::tcp::endpoint ep2(asio::ip::address::from_string("192.168.1.1"), 80);
    manager.addEndpoint(ep2, 3); // 最大3个连接

    // 获取 endpoints
    for (int i = 0; i < 6; ++i) {
        auto endpoint = manager.acquireEndpoint();
        if (endpoint) {
            std::cout << "Acquired endpoint: " << endpoint->address().to_string()
                      << ":" << endpoint->port() << std::endl;

            // 模拟使用后释放
            if (i % 2 == 0) {
                manager.releaseEndpoint(*endpoint);
                std::cout << "Released endpoint" << std::endl;
            }
        } else {
            std::cout << "No available endpoints" << std::endl;
        }
    }

    // 检查状态
    auto stats1 = manager.getEndpointStats(ep1);
    auto stats2 = manager.getEndpointStats(ep2);
    std::cout << "ep1 - Active: " << stats1.second << "/" << stats1.first << std::endl;
    std::cout << "ep2 - Active: " << stats2.second << "/" << stats2.first << std::endl;
}

#include <q1x/net/connection_pool.h>
#include <q1x/net/operation_handler.h>

/**
 * @brief 模拟网络操作处理器，用于测试
 */
class Mock1NetworkHandler : public NetworkOperationHandler<Mock1NetworkHandler> {
public:
    bool handshakeImpl(asio::ip::tcp::socket&)  {
        throw std::runtime_error("Mock connection failure");
    }
    bool keepaliveImpl(asio::ip::tcp::socket&)  { return false; }
    std::chrono::milliseconds timeout() const  { return std::chrono::milliseconds(100); }
    std::chrono::milliseconds check_interval() const  { return std::chrono::seconds(1); }

    void set_timeout(std::chrono::milliseconds timeout)  {
        timeout_ = timeout;
    }
private:
    std::chrono::milliseconds timeout_{};
};

TEST_CASE("TcpConnectionPool connection failure with message check", "[connection_pool]") {
    auto handler = std::make_shared<Mock1NetworkHandler>();
    TcpConnectionPool<Mock1NetworkHandler> pool(2, 5, handler);
    REQUIRE(pool.add_endpoint("127.0.0.1", 7878));

    try {
        auto conn = pool.acquire();
        FAIL("Expected exception not thrown");
    } catch (const std::runtime_error& e) {
        REQUIRE(std::string(e.what()).find("failed") != std::string::npos);
    } catch (...) {
        FAIL("Unexpected exception type");
    }
}

TEST_CASE("TcpConnectionPool basic operations", "[connection_pool]") {
    // 创建模拟网络处理器
    auto handler = std::make_shared<Mock1NetworkHandler>();
    // 创建连接池实例（最小2连接，最大5连接）
    TcpConnectionPool pool(2, 5, handler);

    SECTION("Test endpoint adding") {
        // 测试添加两个endpoint
        REQUIRE(pool.add_endpoint("127.0.0.1", 7878));
        REQUIRE(pool.add_endpoint("192.168.50.158", 7878));

        SECTION("Test connection acquisition with no real server") {
            // 测试从无效endpoint获取连接（应抛出异常）
            REQUIRE_THROWS_AS(pool.acquire(), std::runtime_error);
        }
    }

    SECTION("Test pool lifecycle") {
        // 测试连接池启动和停止
        REQUIRE_NOTHROW(pool.start());
        REQUIRE_NOTHROW(pool.stop());
    }
}

/**
 * @brief 默认网络操作处理器实现
 *
 * 提供符合NetworkOperationHandler接口的具体实现，包含：
 * 1. 基于"hello"握手的连接协议
 * 2. 基于"PING"的心跳检测机制
 * 3. 30秒操作超时控制
 * 4. 5秒心跳检测间隔
 */
class DefaultNetworkHandler : public NetworkOperationHandler<DefaultNetworkHandler> {
public:
    DefaultNetworkHandler() = default;

    bool handshakeImpl(asio::ip::tcp::socket& socket)  {
        return performProtocolOperation(socket, "hello", "hello");
    }

    bool keepaliveImpl(asio::ip::tcp::socket& socket)  {
        return performProtocolOperation(socket, "PING", "PING");
    }

    std::chrono::milliseconds timeout() const  {
        return std::chrono::seconds(30);
    }

    std::chrono::milliseconds check_interval() const  {
        return std::chrono::seconds(5);
    }

    void set_timeout(std::chrono::milliseconds timeout)  {
        timeout_ = timeout;
    }

private:
    std::chrono::milliseconds timeout_;
    bool performProtocolOperation(
            asio::ip::tcp::socket& socket,
            const std::string& request_msg,
            const std::string& expected_response)
    {
        try {
            // 1. 设置带超时的异步操作
            asio::steady_timer timer(socket.get_executor());
            timer.expires_after(timeout());

            std::promise<bool> result_promise;
            std::future<bool> result_future = result_promise.get_future();
            asio::error_code ec;

            // 2. 异步写入请求
            asio::async_write(
                    socket,
                    asio::buffer(request_msg),
                    [&](const asio::error_code& write_ec, size_t) {
                        if (write_ec) {
                            result_promise.set_value(false);
                            return;
                        }

                        // 3. 异步读取响应
                        char response[32] = {0};
                        asio::async_read(
                                socket,
                                asio::buffer(response, expected_response.size()),
                                [&](const asio::error_code& read_ec, size_t len) {
                                    result_promise.set_value(
                                            !read_ec &&
                                            std::string(response, len) == expected_response
                                    );
                                    timer.cancel();
                                });
                    });

            // 4. 设置超时回调
            timer.async_wait([&](const asio::error_code& timer_ec) {
                if (!timer_ec &&
                    result_future.wait_for(std::chrono::seconds(0)) !=
                    std::future_status::ready)
                {
                    socket.cancel(ec); // 忽略cancel错误
                    result_promise.set_value(false);
                }
            });

            // 5. 等待操作完成
            return result_future.get();
        }
        catch (...) {
            return false;
        }
    }
};

/**
 * @brief 模拟服务器用于测试
 */
class MockServer {
public:
    MockServer(unsigned short port, std::string response)
            : acceptor_(io_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
              response_(std::move(response))
    {
        // 使用标准ASIO接口代替底层API
        acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
        start_accept();
        io_thread_ = std::thread([this] { io_.run(); });
    }

private:
    void start_accept() {
        acceptor_.async_accept(
                [this](std::error_code ec, asio::ip::tcp::socket socket) {
                    if (!ec) {
                        // 改用标准ASIO读写接口
                        asio::streambuf buf;
                        asio::read_until(socket, buf, '\n');
                        asio::write(socket, asio::buffer(response_));
                    }
                    start_accept(); // 继续接受新连接
                });
    }

    asio::io_context io_;
    asio::ip::tcp::acceptor acceptor_;
    std::string response_;
    std::thread io_thread_;
};

TEST_CASE("Basic framework test", "[core]") {  // 注意第二个标签参数
    REQUIRE(1 + 1 == 2);
}

// 测试NetworkOperationHandler接口
TEST_CASE("NetworkOperationHandler interface", "[handler]") {
    class MockHandler : public NetworkOperationHandler<MockHandler> {
    public:
        bool handshakeImpl(asio::ip::tcp::socket&)  { return true; }
        bool keepaliveImpl(asio::ip::tcp::socket&)  { return true; }
        std::chrono::milliseconds timeout() const  {
            return std::chrono::seconds(1);
        }
        std::chrono::milliseconds check_interval() const  {
            return std::chrono::seconds(1);
        }

        void set_timeout(std::chrono::milliseconds timeout)  {
            timeout_ = timeout;
        }

    private:
        std::chrono::milliseconds timeout_;
    };

    MockHandler handler;

    SECTION("Timeout value") {
        REQUIRE(handler.timeout().count() == 1000);
    }

    SECTION("Check interval") {
        REQUIRE(handler.check_interval().count() == 1000);
    }
}

// 测试DefaultNetworkHandler
TEST_CASE("DefaultNetworkHandler", "[handler]") {
    DefaultNetworkHandler handler;

    SECTION("Verify timeout") {
        REQUIRE(handler.timeout() == std::chrono::seconds(30));
    }

    SECTION("Verify check interval") {
        REQUIRE(handler.check_interval() == std::chrono::seconds(5));
    }
}

// 测试连接池构造
TEST_CASE("TcpConnectionPool construction", "[pool]") {
    auto handler = std::make_shared<DefaultNetworkHandler>();

    SECTION("Valid construction") {
        SECTION("Normal case") {
            REQUIRE_NOTHROW(TcpConnectionPool(1, 3, handler));
        }
        SECTION("Min equals max") {
            REQUIRE_NOTHROW(TcpConnectionPool(2, 2, handler));
        }
    }

    SECTION("Invalid parameters") {
        SECTION("min > max") {
            REQUIRE_THROWS_AS(
                    TcpConnectionPool(3, 1, handler),
                    std::invalid_argument);
        }

        SECTION("max = 0") {
            REQUIRE_THROWS_AS(
                    TcpConnectionPool(0, 0, handler),
                    std::invalid_argument);
        }

        SECTION("null handler") {
            REQUIRE_THROWS_AS(
                    TcpConnectionPool<DefaultNetworkHandler>(1, 3, nullptr),
                    std::invalid_argument);
        }

        SECTION("min = 0 when max > 0") {
            REQUIRE_NOTHROW(TcpConnectionPool(0, 1, handler));
        }
    }
}

// 测试端点管理
TEST_CASE("Endpoint management", "[pool]") {
    auto handler = std::make_shared<DefaultNetworkHandler>();
    TcpConnectionPool<DefaultNetworkHandler> pool(1, 3, handler);

    SECTION("Add valid endpoints") {
        REQUIRE(pool.add_endpoint("127.0.0.1", 8080));  // IPv4
        REQUIRE(pool.add_endpoint("::1", 8080));        // IPv6
    }

    SECTION("Add invalid endpoints") {
        SECTION("Invalid IP format") {
            REQUIRE_FALSE(pool.add_endpoint("300.300.300.300", 8080));
            REQUIRE_FALSE(pool.add_endpoint("invalid.ip", 8080));
        }

        SECTION("Invalid port") {
            REQUIRE_FALSE(pool.add_endpoint("127.0.0.1", 0));
            //REQUIRE_FALSE(pool.add_endpoint("127.0.0.1", 65536));
        }
    }

    SECTION("Duplicate endpoints") {
        REQUIRE(pool.add_endpoint("10.0.0.1", 8080));
        REQUIRE_FALSE(pool.add_endpoint("10.0.0.1", 8080));
    }
}

// 集成测试

class [[maybe_unused]] EchoServer {
public:
    EchoServer(unsigned short port)
            : acceptor_(io_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {
        start_accept();
        io_thread_ = std::thread([this] { io_.run(); });
    }

    ~EchoServer() {
        io_.stop();
        if (io_thread_.joinable()) io_thread_.join();
    }

private:
    void start_accept() {
        acceptor_.async_accept(
                [this](asio::error_code ec, asio::ip::tcp::socket socket) {
                    if (!ec) handle_client(std::move(socket));
                    start_accept();
                });
    }

    void handle_client(asio::ip::tcp::socket socket) {
        // 简单回显协议
        char buf[256];
        socket.read_some(asio::buffer(buf));

        if (std::string(buf) == "hello") {
            asio::write(socket, asio::buffer("hello"));
        } else if (std::string(buf) == "PING") {
            asio::write(socket, asio::buffer("PING"));
        }
    }

    asio::io_context io_;
    asio::ip::tcp::acceptor acceptor_;
    std::thread io_thread_;
};

TEST_CASE("TcpConnectionPool endpoint validation") {
    auto handler = std::make_shared<DefaultNetworkHandler>();
    TcpConnectionPool<DefaultNetworkHandler> pool(1, 3, handler);

    // 测试无效端口
    REQUIRE_FALSE(pool.add_endpoint("127.0.0.1", 0));      // 端口0无效
    REQUIRE(pool.add_endpoint("127.0.0.1", 65535));  // 端口超出范围

    // 测试有效端口
    REQUIRE(pool.add_endpoint("127.0.0.1", 8080));
    REQUIRE_FALSE(pool.add_endpoint("127.0.0.1", 65535));  // 最大有效端口

    // 测试获取统计信息
    auto stats = pool.getEndpointStats("127.0.0.1", 8080);
    REQUIRE(stats.first > 0);  // 最大连接数
    REQUIRE(stats.second == 0); // 初始活跃连接数应为0
}

using namespace std::chrono_literals; // 添加时间字面量支持, 比如 100ms等等

class MockNetworkHandler : public NetworkOperationHandler<MockNetworkHandler> {
public:
    // 使用统一命名风格（下划线后缀）
    bool handshakeImpl(asio::ip::tcp::socket&)  { return connect_success_; }
    //bool keepalive(asio::ip::tcp::socket&) override { return keepalive_ok_; }
    std::chrono::milliseconds timeout() const  { return std::chrono::milliseconds(100); }
    void set_timeout(std::chrono::milliseconds timeout)  {
        timeout_ = timeout;
    }
    //std::chrono::milliseconds check_interval() const override { return std::chrono::milliseconds(200); }

    // 成员变量明确标记为可修改状态
    bool connect_success_ = true;
    bool keepalive_ok_ = true;
    // 使用标准接口实现
    bool keepaliveImpl(asio::ip::tcp::socket&)  {
        return keepalive_result;
    }

    // 控制Mock行为的公有成员
    bool keepalive_result = true;
    std::chrono::milliseconds check_interval() const  {
        return 100ms; // 缩短测试间隔
    }
private:
    std::chrono::milliseconds timeout_;
};

TEST_CASE("TcpConnectionPool endpoint validation", "[pool][validation]") {
    asio::io_context io;
    auto handler = std::make_shared<MockNetworkHandler>();
    TcpConnectionPool pool(5, 10, handler);

    SECTION("Valid port range") {
        REQUIRE(pool.add_endpoint("127.0.0.1", 1));      // 最小有效端口
        REQUIRE(pool.add_endpoint("127.0.0.1", 65535));  // 最大有效端口
    }

    SECTION("Invalid port range") {
        REQUIRE_FALSE(pool.add_endpoint("127.0.0.1", 0));     // 端口0无效
        //REQUIRE_FALSE(pool.add_endpoint("127.0.0.1", 65536)); // 溢出测试
    }
}

TEST_CASE("TcpConnectionPool timeout scenarios", "[pool][timeout]") {
    //asio::io_context io;
    auto handler = std::make_shared<MockNetworkHandler>();
    TcpConnectionPool pool(5, 10, handler);
    REQUIRE(pool.add_endpoint("127.0.0.1", 8080));

    SECTION("Immediate timeout") {
        handler->set_timeout(1ms); // 1毫秒超时
        handler->connect_success_ = false;

        REQUIRE_THROWS_MATCHES(
                pool.acquire(),
                std::runtime_error,
                Catch::Matchers::Message("Connection timed out")
        );
    }

    SECTION("Normal connection with timeout") {
        handler->set_timeout(1000ms); // 1秒超时
        handler->connect_success_ = true;

        REQUIRE_NOTHROW(pool.acquire());
    }
}

TEST_CASE("TcpConnectionPool connection lifecycle", "[pool][lifecycle]") {
    //asio::io_context io;
    auto handler = std::make_shared<MockNetworkHandler>();
    TcpConnectionPool pool(5, 10, handler);
    REQUIRE(pool.add_endpoint("127.0.0.1", 7878));

    SECTION("Successful connection") {
        handler->connect_success_ = true;  // 使用正确的成员变量名
        auto conn = pool.acquire();
        REQUIRE(conn != nullptr);
    }

    SECTION("Failed connection") {
        handler->connect_success_ = false;
        REQUIRE_THROWS_AS(pool.acquire(), std::runtime_error);
    }
}

//class TcpConnectionPoolTester {
//public:
//    // 通过公有方法间接测试
//    static void triggerHealthCheck(TcpConnectionPool& pool) {
//        // 通过start()触发检查（假设start()是公有方法）
//        pool.start();
//        std::this_thread::sleep_for(100ms); // 等待检查完成
//        pool.stop();
//    }
//
//    // 通过观察副作用验证结果
//    static bool containsConnection(TcpConnectionPool& pool,
//                                   const asio::ip::tcp::endpoint& ep) {
//        try {
//            auto stats = pool.getEndpointStats(ep.address().to_string(), ep.port());
//            return stats.second > 0; // active_connections > 0
//        } catch (...) {
//            return false;
//        }
//    }
//};

//TEST_CASE("Connection health management", "[pool][health]") {
//    asio::io_context io;
//    auto handler = std::make_shared<MockNetworkHandler>();
//    TcpConnectionPool pool(5, 10, handler);
//
//    // 1. 添加测试端点
//    REQUIRE(pool.add_endpoint("127.0.0.1", 7878));
//
//    // 2. 获取连接使其成为active状态
//    auto conn = pool.acquire();
//    auto endpoint = conn->socket().remote_endpoint();
//
//    SECTION("Healthy connections remain") {
//        // 设置Mock返回健康状态
//        handler->keepalive_result = true;
//
//        TcpConnectionPoolTester::triggerHealthCheck(pool);
//        REQUIRE(TcpConnectionPoolTester::containsConnection(pool, endpoint));
//    }
//
////    SECTION("Unhealthy connections are removed") {
////        // 设置Mock返回不健康状态
////        handler->keepalive_result = false;
////
////        TcpConnectionPoolTester::triggerHealthCheck(pool);
////        REQUIRE_FALSE(TcpConnectionPoolTester::containsConnection(pool, endpoint));
////    }
//}

// 测试专用Echo服务器
class TestEchoServer {
public:
    TestEchoServer(unsigned short port)
            : acceptor_(io_context_, {asio::ip::tcp::v4(), port}) {
        accept();
    }

    void start() { io_context_.run(); }
    void stop() { io_context_.stop(); }

private:
    void accept() {
        acceptor_.async_accept([this](asio::error_code ec, asio::ip::tcp::socket socket) {
            if (!ec) {
                // 模拟心跳响应
                asio::write(socket, asio::buffer("PONG"));
                accept();
            }
        });
    }

    asio::io_context io_context_;
    asio::ip::tcp::acceptor acceptor_;
};

class TestNetworkHandler : public NetworkOperationHandler<TestNetworkHandler> {
public:
    bool handshakeImpl(asio::ip::tcp::socket&)  { return true; }
    bool keepaliveImpl(asio::ip::tcp::socket& sock)  {
        asio::write(sock, asio::buffer("PING"));
        return true;
    }
    [[nodiscard]] std::chrono::milliseconds timeout() const  { return 5s; }
    void set_timeout(std::chrono::milliseconds)  {}
    [[nodiscard]] std::chrono::milliseconds check_interval() const  { return 30s; }
};

TEST_CASE("Non-blocking heartbeat test", "[.integration]") {
    // 1. 安全启动服务器
    asio::io_context io_ctx;
    asio::ip::tcp::acceptor acceptor(io_ctx, {asio::ip::tcp::v4(), 54321});
    std::promise<void> server_stopped;

    std::thread server_thread([&]() {
        asio::ip::tcp::socket sock(io_ctx);
        acceptor.async_accept(sock, [&](asio::error_code ec) {
            if (!ec) asio::write(sock, asio::buffer("PONG"));
        });

        io_ctx.run(); // 非阻塞运行
        server_stopped.set_value(); // 通知完成
    });

    // 2. 初始化连接池
    auto handler = std::make_shared<TestNetworkHandler>();
    TcpConnectionPool pool(3, 5, handler);
    REQUIRE(pool.add_endpoint("127.0.0.1", 54321));

    // 3. 运行测试（10秒演示）
    pool.start();
    auto test_future = server_stopped.get_future();
    REQUIRE(test_future.wait_for(10s) != std::future_status::ready); // 验证服务器未提前退出

    // 4. 异步清理
    io_ctx.stop();
    pool.stop();

    // 5. 非阻塞等待线程结束（最多等待1秒）
    if (server_thread.joinable()) {
        server_thread.detach(); // 或使用更精细的超时控制
        // 替代方案：if (server_thread.joinable()) server_thread.join();
    }
}

TEST_CASE("Test-app", "[net]") {
    auto handler = std::make_shared<TestNetworkHandler>();
    TcpConnectionPool pool(3, 5, handler);
    // 添加多个endpoint
    pool.add_endpoint("127.0.0.1", 7878);
    pool.add_endpoint("192.168.50.158", 7878);
    constexpr int THREAD_NUM = 2;
    constexpr int REQ_PER_THREAD = 10;
    std::vector<std::thread> workers;
    std::atomic<int> success_count{0};
    std::mutex cout_mutex;

    // 工作线程函数
    auto worker_task = [&] {
        for (int i = 0; i < REQ_PER_THREAD; ++i) {
            auto conn = pool.acquire();
            if (conn) {
                try {
                    // 构造请求信息: 线程ID+请求编号
                    std::ostringstream oss;
                    oss << "REQ from thread:" << std::this_thread::get_id()
                        << " seq:" << i;
                    const std::string req = oss.str();
                    asio::ip::tcp::socket& sock  = conn->socket();
                    // 发送
                    asio::write(sock, asio::buffer(req));

                    // 接收
                    // 准备接收缓冲区
                    std::vector<uint8_t> recv_buffer(1024); // 预分配1KB空间
                    size_t bytes_received = asio::read(
                            sock,
                            asio::buffer(recv_buffer),
                            asio::transfer_at_least(1) // 至少接收1字节
                    );
                    auto resp = std::string(reinterpret_cast<const char *>(recv_buffer.data()), bytes_received);

                    // 验证响应匹配请求
                    if (resp == req) {
                        ++success_count;
                        std::lock_guard<std::mutex> lock(cout_mutex);
                        std::cout << "√ Success: " << resp << "\n";
                    }
                } catch (...) {
                    std::lock_guard<std::mutex> lock(cout_mutex);
                    spdlog::warn("× Request failed");
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(50)); // 模拟处理延时
        }
    };

    // 创建线程
    for (int i = 0; i < THREAD_NUM; ++i) {
        workers.emplace_back(worker_task);
    }

    // 等待所有线程完成
    for (auto &t: workers) {
        t.join();
    }

    std::this_thread::sleep_for(std::chrono::seconds(60));
}