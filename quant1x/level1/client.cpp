#include <quant1x/level1/client.h>
#include <quant1x/exchange/session.h>

namespace level1 {
    namespace {
        std::unique_ptr<TcpConnectionPool<ProtocolHandler>> tdx_connection_pool() {
            namespace fs = std::filesystem;
            auto _handler = std::make_shared<ProtocolHandler>();
            std::string cache_server_filename = config::get_meta_path() + "/tdx.bin";
            bool need_update = false;
            if (!fs::exists(cache_server_filename) || fs::file_size(cache_server_filename) == 0) {
                need_update = true;
            }
            auto modified = io::last_modified_time(cache_server_filename);
            if (!need_update) {
                need_update = exchange::can_initialize(modified);
            }
            size_t concurrency = 10;
            if (need_update) {
                cista::offset::vector<level1::Server> servers = level1::detect();
                auto buf = cista::serialize(servers);
                std::ofstream out(cache_server_filename, std::ios::binary | std::ios::out | std::ios::trunc);
                out.write(reinterpret_cast<char *>(buf.data()), buf.size());
                concurrency = std::min(concurrency, static_cast<size_t>(servers.size()));
            }
            auto tcpConnectionPool = std::make_unique<TcpConnectionPool<ProtocolHandler>>(1, concurrency, _handler);
            {
                // 从文件读取并反序列化
                std::ifstream in(cache_server_filename, std::ios::binary);
                std::vector<char> file_buf((std::istreambuf_iterator<char>(in)),
                                           std::istreambuf_iterator<char>());
                //std::vector<u8> buf;
                //buf.insert(file_buf.data(), file_buf.size());
                auto deserialized = cista::deserialize<cista::offset::vector<level1::Server>>(file_buf);

                for (auto d: *deserialized) {
                    spdlog::debug("{}: [{}:{}]", d.Name, d.Host, d.Port);
                    tcpConnectionPool->add_endpoint(d.Host.str(), d.Port);
                }
            }
            return tcpConnectionPool;
        }

        std::once_flag _init_tcp_connection_pool;
        std::unique_ptr<TcpConnectionPool<ProtocolHandler>> _connection_pool_ptr = nullptr;
    }

    std::unique_ptr<Connection, std::function<void(Connection*)>> client() {
        std::call_once(_init_tcp_connection_pool, []() {
            try {
                _connection_pool_ptr = tdx_connection_pool();
            } catch (const std::exception& e) {
                spdlog::error("连接池初始化失败: {}", e.what());
                // 可抛出异常或设置默认连接池
            }
        });
        return _connection_pool_ptr->acquire();
    }
} // namespace level1