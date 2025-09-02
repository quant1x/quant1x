#pragma once
#ifndef API_NET_OPERATION_HANDLER_H
#define API_NET_OPERATION_HANDLER_H 1

#include "quant1x/net/base.h"

// 基础网络操作接口(CRTP模板基类)
template <typename Derived>
class NetworkOperationHandler {
public:
    //virtual ~NetworkOperationHandler() = default;

    /**
     * @brief 网络连接建立后的协议握手
     * @param socket 新建立的socket连接
     * @return 是否握手成功
     */
    bool handshake(asio::ip::tcp::socket& socket) {
        return static_cast<Derived*>(this)->handshakeImpl(socket);
    }

    /**
     * @brief 连接保活检测
     * @param socket 需要检测的连接
     * @return 连接是否仍然有效
     */
    bool keepalive(asio::ip::tcp::socket& socket) {
        return static_cast<Derived*>(this)->keepaliveImpl(socket);
    }

    /**
     * @brief 所有网络操作的统一超时时间
     */
    std::chrono::milliseconds timeout() {
        return this->timeout_;
    };

    /**
     * @brief 设置超时时间
     * @param timeout 需要检测的连接
     */
    void set_timeout(std::chrono::milliseconds timeout){
        this->timeout_ = timeout;
    }

    /**
     * @brief 保活检测的执行间隔
     */
    std::chrono::milliseconds check_interval() {
        return this->interval_;
    };

private:
    std::chrono::milliseconds interval_ = std::chrono::seconds(5); ///< 默认保活时间间隔5秒
    std::chrono::milliseconds timeout_ = std::chrono::seconds(10); ///< 默认超时10秒
};

#endif //API_NET_OPERATION_HANDLER_H
