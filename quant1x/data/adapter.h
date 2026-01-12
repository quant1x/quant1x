#pragma once
#ifndef QUANT1X_DATA_ADAPTER_H
#define QUANT1X_DATA_ADAPTER_H 1

#include <quant1x/std/api.h>
#include <quant1x/exchange/timestamp.h>
#include <vector>
#include <string>
#include <cstdint>
#include <memory>

namespace data {

    using Kind = uint64_t;

    constexpr Kind PluginMaskBaseData = 0x1000000000000000; // 基础数据
    constexpr Kind PluginMaskFeature  = 0x2000000000000000; // 特征数据
    constexpr Kind PluginMaskStrategy = 0x3000000000000000; // 策略

    const std::string DefaultDataProvider = "quant1x";

    // 缓存的概要信息
    class Schema {
    public:
        virtual ~Schema() = default;
        virtual data::Kind Kind() const = 0; // Kind 数据类型
        virtual std::string Owner() = 0; // Owner 提供者
        virtual std::string Key() const = 0; // Key 数据关键词, key与cache落地强关联
        virtual std::string Name() const = 0; // Name 特性名称
        virtual std::string Usage() const = 0; // Usage 控制台参数提示信息, 数据描述(data description)
    };

    // 基础数据适配器
    class DataAdapter : public Schema {
    public:
        virtual ~DataAdapter() = default;
        // 控制台打印
        virtual void Print(const std::string& code, const std::vector<exchange::timestamp>& dates = {}) = 0;
        // 更新数据
        virtual void Update(const std::string& code, const exchange::timestamp& date = 0) = 0;
    };

    // 特征数据适配器
    class FeatureAdapter : public DataAdapter {
    public:
        // 特征数据为聚合文件路径
        std::string Filename(const exchange::timestamp &timestamp = 0);
        virtual void init(const exchange::timestamp &timestamp) = 0;
        virtual std::unique_ptr<FeatureAdapter> clone() const = 0;
        virtual std::vector<std::string> headers() const = 0;
        virtual std::vector<std::string> values() const = 0;
    };

    void Register(std::unique_ptr<DataAdapter> plugin);

    class ErrAlreadyExists : public std::runtime_error {
    public:
        ErrAlreadyExists() : std::runtime_error("the plugin already exists") {}
    };

    template<typename T>
    class PluginRegistrar {
    public:
        PluginRegistrar() {
            // 利用多态获取 Kind
            std::unique_ptr<DataAdapter> plugin = std::make_unique<T>();
            Register(std::move(plugin));
        }
    };

    #define REGISTER_PLUGIN(cls)            \
        namespace {                         \
            cache::PluginRegistrar<cls> cls##Registrar; \
        }

    std::vector<DataAdapter*> PluginsWithName(Kind pluginType, const std::vector<std::string>& keywords);
    std::vector<DataAdapter*> Plugins(Kind mask = 0);

} // namespace data

#endif // QUANT1X_DATA_ADAPTER_H
