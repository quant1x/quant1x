#include <quant1x/level1/client.h>
#include <quant1x/exchange/session.h>
#include <quant1x/std/except.h>
#include <quant1x/encoding/yaml.h>

namespace level1 {
    struct ServerList {
        std::vector<ServerInfo> servers;
    };

        
    namespace {
        std::unique_ptr<TcpConnectionPool<StandardProtocolHandler> > standard_protocol_connection_pool() {
            namespace fs = std::filesystem;
            auto _handler = std::make_shared<StandardProtocolHandler>();
            std::string cache_server_filename = config::get_meta_path() + "/server.bin";
            bool need_update = false;
            if (!fs::exists(cache_server_filename) || fs::file_size(cache_server_filename) == 0) {
                need_update = true;
            }
            if (!need_update) {
                auto modified = io::last_modified_time(cache_server_filename);
                need_update = exchange::can_initialize(modified);
            }
            size_t concurrency = 10;
            if (need_update) {
                std::vector<level1::ServerInfo> servers = level1::detect();
                ServerList server_list{};
                server_list.servers = servers;
                ::encoding::save_yaml(server_list, cache_server_filename);
                //out.write(reinterpret_cast<char *>(buf.data()), buf.size());
                concurrency = std::min(concurrency, static_cast<size_t>(servers.size()));
            }
            auto tcpConnectionPool = std::make_unique<TcpConnectionPool<StandardProtocolHandler> >(1, concurrency, _handler); {
                // 从文件读取并反序列化
                auto [servers] = ::encoding::load_yaml<ServerList>(cache_server_filename);
                for (auto d: servers) {
                    spdlog::debug("{}: [{}:{}]", d.Name, d.Host, d.Port);
                    tcpConnectionPool->add_endpoint(d.Host, d.Port);
                }
            }
            return tcpConnectionPool;
        }

        std::once_flag _init_tcp_connection_pool;
        std::unique_ptr<TcpConnectionPool<StandardProtocolHandler> > _connection_pool_ptr = nullptr;
    }

    std::unique_ptr<Connection, std::function<void(Connection *)> > client() {
        std::call_once(_init_tcp_connection_pool, []() {
            try {
                _connection_pool_ptr = standard_protocol_connection_pool();
            } catch (const std::exception &e) {
                spdlog::error("连接池初始化失败: {}", e.what());
                // 可抛出异常或设置默认连接池
                throw e;
            }
        });
        return _connection_pool_ptr->acquire();
    }
} // namespace level1
