#pragma once
#ifndef QUANT1X_NETWORK_IO_ENDPOINT_H
#define QUANT1X_NETWORK_IO_ENDPOINT_H 1

#include "quant1x/net/base.h"
#include "quant1x/runtime/core.h"

class EndpointManager {
public:
    /**
     * @brief 添加或更新 endpoint 配置
     * @param ip ip地址或域名
     * @param port 端口
     * @param max_connections 该 endpoint 允许的最大连接数
     * @return 返回是否添加成功
     */
    bool addEndpoint(const std::string &ip, u16 port, size_t max_connections) {
        // 首先验证端口有效性
        if (port == 0 || port >= 65535) {
            return false;
        }

        try {
            // 使用ASIO严格验证IP地址格式
            asio::ip::address addr = asio::ip::make_address(ip);
            asio::ip::tcp::endpoint ep(addr, port);
            return addEndpoint(ep, max_connections);
        }
        catch (const std::exception& e) {
            spdlog::error("[endpoint] - 标准异常: {} (type: {})", e.what(), typeid(e).name());
            // 捕获无效IP地址导致的异常
            return false;
        }
    }

    /**
     * @brief 添加或更新 endpoint 配置
     * @param endpoint TCP endpoint
     * @param max_connections 该 endpoint 允许的最大连接数
     * @return 返回是否添加成功
     */
    bool addEndpoint(const asio::ip::tcp::endpoint& endpoint, size_t max_connections) {
        std::lock_guard<std::mutex> lock(mutex_);

        // 检查是否已存在相同endpoint
        if (endpoints_data_.find(endpoint) != endpoints_data_.end()) {
            return false;
        }

        // 添加新endpoint
        endpoints_list_.push_back(endpoint);
        endpoints_data_[endpoint] = {max_connections, 0};
        return true;
    }

    /**
     * @brief 移除 endpoint
     * @param endpoint 要移除的 endpoint
     */
    void removeEndpoint(const asio::ip::tcp::endpoint& endpoint) {
        std::lock_guard<std::mutex> lock(mutex_);
        endpoints_data_.erase(endpoint);
        endpoints_list_.erase(
                std::remove(endpoints_list_.begin(), endpoints_list_.end(), endpoint),
                endpoints_list_.end()
        );
    }

    /**
     * @brief 获取一个可用的 endpoint
     * @return 可用的 endpoint，如果没有可用则返回空的 optional
     */
    std::optional<asio::ip::tcp::endpoint> acquireEndpoint() {
        std::lock_guard<std::mutex> lock(mutex_);

//        // 随机打乱 endpoints 顺序以实现简单负载均衡
//        std::vector<asio::ip::tcp::endpoint> shuffled_list = endpoints_list_;
//        std::shuffle(shuffled_list.begin(), shuffled_list.end(), std::mt19937{std::random_device{}()});

        for (const auto& endpoint : endpoints_list_) {
            auto& data = endpoints_data_[endpoint];
            if (data.active_connections < data.max_connections) {
                data.active_connections++;
                spdlog::debug("acquire endpoint: {}", endpoint);
                return endpoint;
            }
        }

        return std::nullopt;
    }

    /**
     * @brief 释放 endpoint 的使用权
     * @param endpoint 要释放的 endpoint
     */
    void releaseEndpoint(const asio::ip::tcp::endpoint& endpoint) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = endpoints_data_.find(endpoint);
        if (it != endpoints_data_.end() && it->second.active_connections > 0) {
            it->second.active_connections--;
        }
        spdlog::debug("release endpoint: {}", endpoint);
    }

    /**
     * @brief 获取 endpoint 的当前使用情况
     * @param endpoint 查询的 endpoint
     * @return pair<最大连接数, 当前活跃连接数>
     * @throws std::out_of_range 如果 endpoint 不存在
     */
    std::pair<size_t, size_t> getEndpointStats(const asio::ip::tcp::endpoint& endpoint) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = endpoints_data_.find(endpoint);
        if (it == endpoints_data_.end()) {
            throw std::out_of_range("Endpoint not found");
        }
        return {it->second.max_connections, it->second.active_connections};
    }

    /**
     * @brief 获取所有 endpoints
     * @return 所有管理的 endpoints 列表
     */
    std::vector<asio::ip::tcp::endpoint> getAllEndpoints() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return endpoints_list_;
    }

    /**
     * @brief 获取所有 endpoints 的剩余可用资源总数
     * @return 所有 endpoints 剩余的可用连接数总和
     */
    size_t getAvailableResources() const {
        std::lock_guard<std::mutex> lock(mutex_);
        size_t available_resources = 0;

        for (const auto& [endpoint, data] : endpoints_data_) {
            if (data.active_connections < data.max_connections) {
                available_resources += (data.max_connections - data.active_connections);
            }
        }

        return available_resources;
    }

private:
    struct EndpointData {
        size_t max_connections;
        size_t active_connections;
    };

    std::vector<asio::ip::tcp::endpoint> endpoints_list_;
    std::unordered_map<asio::ip::tcp::endpoint, EndpointData> endpoints_data_;
    mutable std::mutex mutex_;
};

#endif //QUANT1X_NETWORK_IO_ENDPOINT_H
