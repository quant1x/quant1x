#include <quant1x/contrib/data/tdx/client.h>
#include <quant1x/test/test.h>

namespace tdx = quant1x::contrib::data::tdx;

TEST_CASE("detect", "[level1]") {
    auto list = tdx::detect();
    for (auto const &v : list) {
        std::cout << v.Host << "," << v.latency_ms << std::endl;
    }
}

TEST_CASE("client", "[level1]") {
    auto client = tdx::get_std_conn();

}

#include <yaml-cpp/yaml.h>

#include <boost/pfr.hpp>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

// 服务器配置结构体
struct ServerConfig {
    std::string name;        ///< 服务器名
    std::string host;        ///< 主机名/ip
    u16         port;        ///< 端口好
    i64         latency_ms;  ///< 延迟
};

// 应用配置(包含服务器列表)
struct AppConfig {
    std::vector<ServerConfig> servers;
};

// 类型特征: 判断是否为 vector
template <typename T>
struct is_vector : std::false_type {};

template <typename T, typename A>
struct is_vector<std::vector<T, A>> : std::true_type {};

template <typename T>
constexpr bool is_vector_v = is_vector<T>::value;

// YAML 反序列化器(修复 string_view 问题)
class YamlDeserializer {
public:
    template <typename T>
    static T deserialize(const YAML::Node &node) {
        T obj{};
        deserialize_to_object(node, obj);
        return obj;
    }

private:
    // 主要: 将 YAML 节点映射到结构体对象
    template <typename T>
    static void deserialize_to_object(const YAML::Node &node, T &obj) {
        boost::pfr::for_each_field(obj, [&](auto &field, std::size_t idx) {
            const auto &field_names = boost::pfr::names_as_array<T>();
            const auto field_name = std::string(field_names[idx]);  // 修正点
            if (node[field_name]) {
                deserialize_field(node[field_name], field);
            }
        });
    }

    // 基础字段解析(递归支持嵌套结构体, 容器等)
    template <typename T>
    static void deserialize_field(const YAML::Node &node, T &field) {
        if constexpr (std::is_same_v<T, std::string>) {
            field = node.as<std::string>();
        } else if constexpr (std::is_arithmetic_v<T> || std::is_same_v<T, bool>) {
            field = node.as<T>();
        } else if constexpr (is_vector_v<T>) {
            deserialize_vector(node, field);
        } else {
            // 假设是嵌套结构体, 递归处理
            deserialize_to_object(node, field);
        }
    }

    // 解析 vector 容器
    template <typename T>
    static void deserialize_vector(const YAML::Node &node, T &vec) {
        using ValueType = typename T::value_type;
        vec.clear();
        for (const auto &item : node) {
            ValueType value{};
            deserialize_field(item, value);
            vec.push_back(value);
        }
    }
};

// ================================================
// ✅ 对称的 YAML 序列化/反序列化器
// ================================================
class YamlSerializer {
public:
    // 反序列化: YAML → struct
    template<typename T>
    static T deserialize(const YAML::Node& node) {
        T obj{};
        deserialize_to_object(node, obj);
        return obj;
    }

    // 序列化: struct → YAML
    template<typename T>
    static YAML::Node serialize(const T& obj) {
        return serialize_to_node(obj);
    }

private:
    // ========== 反序列化 ==========
    template<typename T>
    static void deserialize_to_object(const YAML::Node& node, T& obj) {
        boost::pfr::for_each_field(obj, [&](auto& field, std::size_t idx) {
            const auto field_name = get_field_name<T>(idx);
            if (node[field_name]) {
                deserialize_field(node[field_name], field);
            }
        });
    }

    template<typename T>
    static void deserialize_field(const YAML::Node& node, T& field) {
        if constexpr (std::is_same_v<T, std::string>) {
            field = node.as<std::string>();
        } else if constexpr (std::is_arithmetic_v<T> || std::is_same_v<T, bool>) {
            field = node.as<T>();
        } else if constexpr (is_vector_v<T>) {
            deserialize_vector(node, field);
        } else {
            deserialize_to_object(node, field);
        }
    }

    template<typename T>
    static void deserialize_vector(const YAML::Node& node, T& vec) {
        using ValueType = typename T::value_type;
        vec.clear();
        for (const auto& item : node) {
            ValueType value{};
            deserialize_field(item, value);
            vec.push_back(value);
        }
    }

    // ========== 序列化 ==========
    template<typename T>
    static YAML::Node serialize_to_node(const T& obj) {
        YAML::Node node;
        boost::pfr::for_each_field(obj, [&](const auto& field, std::size_t idx) {
            const auto field_name = get_field_name<T>(idx);
            serialize_field(node[field_name], field);
        });
        return node;
    }

    template<typename T>
    static void serialize_field(YAML::Node& node, const T& field) {
        if constexpr (std::is_same_v<T, std::string> ||
                     std::is_arithmetic_v<T> || std::is_same_v<T, bool>) {
            node = field;
        } else if constexpr (is_vector_v<T>) {
            serialize_vector(node, field);
        } else {
            node = serialize_to_node(field);
        }
    }

    template<typename T>
    static void serialize_vector(YAML::Node& node, const T& vec) {
        for (const auto& item : vec) {
            YAML::Node item_node;
            serialize_field(item_node, item);
            node.push_back(item_node);
        }
    }

    // ========== 公共辅助函数 ==========
    template<typename T>
    static std::string_view get_field_name(std::size_t idx) {
        return boost::pfr::names_as_array<T>()[idx];
    }
};

TEST_CASE("ParseServerListFromYAML", "[server-config]") {
    std::string yaml_content = R"(
servers:
  - name: "web-server-01"
    host: "192.168.1.10"
    port: 8080
    ssl_enabled: true
    tags: ["production", "web"]

  - name: "db-server-01"
    host: "192.168.1.20"
    port: 5432
    ssl_enabled: false
    tags: ["production", "database"]

  - name: "cache-server-01"
    host: "192.168.1.30"
    port: 6379
    ssl_enabled: false
    tags: ["staging", "cache"]
)";

    YAML::Node root   = YAML::Load(yaml_content);
    auto       config = YamlDeserializer::deserialize<AppConfig>(root);

    REQUIRE(config.servers.size() == 3);

    SECTION("First server") {
        auto &s = config.servers[0];
        REQUIRE(s.name == "web-server-01");
        REQUIRE(s.host == "192.168.1.10");
        REQUIRE(s.port == 8080);
    }

    SECTION("Second server") {
        auto &s = config.servers[1];
        REQUIRE(s.name == "db-server-01");
        REQUIRE(s.host == "192.168.1.20");
        REQUIRE(s.port == 5432);
    }

    // 打印验证
    for (const auto &server : config.servers) {
        std::cout << "Server: " << server.name << " @ " << server.host << ":" << server.port << ")";
        std::cout << "\n\n";
    }
}
