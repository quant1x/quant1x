#pragma once
#ifndef QUANT1X_NETWORK_IO_ENDPOINT_H
#define QUANT1X_NETWORK_IO_ENDPOINT_H 1

#include <quant1x/io/base.h>
#include <quant1x/runtime/core.h>

class EndpointManager {
public:
    /**
     * @brief 添加网络端点
     *
     * 验证并添加一个TCP/IP端点, 包含IP地址和端口的有效性检查
     *
     * @param ip 要添加的IP地址字符串, 支持IPv4/IPv6格式
     * @param port 端口号, 范围1-65534
     * @param max_connections 该端点允许的最大连接数
     * @return true 添加成功
     * @return false 添加失败(端口无效或IP格式错误)
     * @throws 无显式抛出, 但会捕获并处理asio::ip::make_address可能抛出的异常
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
     * @brief 添加一个新的TCP端点
     *
     * 将指定的TCP端点添加到端点列表中, 并设置其最大连接数限制. 
     *
     * @param endpoint 要添加的TCP端点
     * @param max_connections 该端点允许的最大连接数
     * @return bool 添加成功返回true, 如果端点已存在则返回false
     * @note 此操作是线程安全的, 内部使用互斥锁保护
     */
    bool addEndpoint(const asio::ip::tcp::endpoint &endpoint, size_t max_connections) {
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
     * @brief 移除指定的TCP端点
     *
     * 从端点数据集合和端点列表中移除指定的TCP端点. 此操作是线程安全的. 
     *
     * @param endpoint 要移除的TCP端点
     * @note 此函数会获取互斥锁以保证线程安全
     */
    void removeEndpoint(const asio::ip::tcp::endpoint &endpoint) {
        std::lock_guard<std::mutex> lock(mutex_);
        endpoints_data_.erase(endpoint);
        endpoints_list_.erase(
                std::remove(endpoints_list_.begin(), endpoints_list_.end(), endpoint),
                endpoints_list_.end()
        );
    }

    /**
     * @brief 从可用端点列表中获取一个可用的TCP端点
     *
     * 该方法会遍历所有已配置的端点, 返回第一个当前活跃连接数未达到最大限制的端点. 
     * 获取成功后, 该端点的活跃连接数会自动增加. 
     *
     * @return std::optional<asio::ip::tcp::endpoint> 返回可用的端点, 如果没有可用端点则返回std::nullopt
     * @note 此方法是线程安全的, 内部使用互斥锁保护共享数据
     * @see endpoints_list_ 端点列表
     * @see endpoints_data_ 端点状态数据
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
     * @brief 释放指定端点的连接计数
     *
     * 减少指定端点上的活跃连接计数. 该操作是线程安全的, 
     * 会在内部使用互斥锁保护共享数据. 
     *
     * @param endpoint 要释放连接的端点对象
     *
     * @note 如果端点不存在或当前没有活跃连接, 则不做任何操作
     * @note 操作完成后会记录调试日志
     */
    void releaseEndpoint(const asio::ip::tcp::endpoint &endpoint) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto                        it = endpoints_data_.find(endpoint);
        if (it != endpoints_data_.end() && it->second.active_connections > 0) {
            it->second.active_connections--;
        }
        spdlog::debug("release endpoint: {}", endpoint);
    }

    /**
     * @brief 获取指定端点的连接统计信息
     *
     * @param endpoint 要查询的TCP端点
     * @return std::pair<size_t, size_t> 返回一个pair, 包含最大连接数和当前活跃连接数
     * @throws std::out_of_range 如果端点不存在于统计数据中
     */
    std::pair<size_t, size_t> getEndpointStats(const asio::ip::tcp::endpoint &endpoint) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto                        it = endpoints_data_.find(endpoint);
        if (it == endpoints_data_.end()) {
            throw std::out_of_range("Endpoint not found");
        }
        return {it->second.max_connections, it->second.active_connections};
    }

    /**
     * @brief 获取所有TCP端点列表
     *
     * 该函数线程安全地返回当前存储的所有TCP端点列表. 
     *
     * @return std::vector<asio::ip::tcp::endpoint> 包含所有TCP端点的vector
     * @note 该函数通过互斥锁保证线程安全
     */
    std::vector<asio::ip::tcp::endpoint> getAllEndpoints() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return endpoints_list_;
    }

    /**
     * @brief 获取当前可用的资源总数
     *
     * 遍历所有端点数据, 计算每个端点剩余可用连接数的总和. 
     * 该操作是线程安全的, 内部使用互斥锁保护. 
     *
     * @return size_t 返回当前系统可用的总资源数(剩余连接数总和)
     */
    size_t getAvailableResources() const {
        std::lock_guard<std::mutex> lock(mutex_);
        size_t                      available_resources = 0;

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
