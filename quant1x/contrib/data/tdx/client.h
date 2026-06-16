#pragma once
#ifndef QUANT1X_LEVEL1_CLIENT_H
#define QUANT1X_LEVEL1_CLIENT_H 1

#include <quant1x/io/connection_pool.h>
#include <quant1x/std/util.h>
#include <quant1x/std/api.h>
#include <quant1x/encoding/charsets.h>
#include <quant1x/std/buffer.h>

#include <quant1x/data/meta/exchange.h>
#include <quant1x/data/meta/session.h>
#include <quant1x/contrib/data/tdx/protocol.h>
#include <quant1x/contrib/data/tdx/helpers.h>
#include <quant1x/contrib/data/tdx/level1/hello1.h>
#include <quant1x/contrib/data/tdx/level1/hello2.h>
#include <quant1x/contrib/data/tdx/level1/heartbeat.h>
#include <quant1x/contrib/data/tdx/level1/xdxr_info.h>
#include <quant1x/contrib/data/tdx/level1/finance_info.h>
#include <quant1x/contrib/data/tdx/level1/security_count.h>
#include <quant1x/contrib/data/tdx/level1/security_list.h>
#include <quant1x/contrib/data/tdx/level1/security_quote.h>
#include <quant1x/contrib/data/tdx/level1/security_bars.h>
#include <quant1x/contrib/data/tdx/level1/transaction_data.h>
#include <quant1x/contrib/data/tdx/level1/transaction_history.h>
#include <quant1x/contrib/data/tdx/level1/block_meta.h>
#include <quant1x/contrib/data/tdx/level1/block_info.h>
#include <quant1x/contrib/data/tdx/level1/minute_time.h>
#include <quant1x/contrib/data/tdx/level1/config.h>

namespace level1 {

    /// 网络协议
    #pragma pack(push, 1)  // 确保1字节对齐

    #pragma pack(pop)  // 恢复默认对齐方式

    class StandardProtocolHandler : public NetworkOperationHandler<StandardProtocolHandler> {
    public:
        bool handshakeImpl(asio::ip::tcp::socket &socket) {
            try {
                // 第一次协议握手
                Hello1 hello1;
                process(socket, hello1);
                // 第二次协议握手
                Hello2 hello2;
                process(socket, hello2);
                return true;
            } catch (const std::bad_cast& e) {
                spdlog::error("Cannot cast: {}", e.what());
                return false;
            } catch (...) {
                return false;
            }
        }

        bool keepaliveImpl(asio::ip::tcp::socket &socket) {
            try {
                // 心跳检测
                Heartbeat hb;
                process(socket, hb);
                return true;
            } catch (...) {
                return false;
            }
        }
    };

    /**
     * @brief 获取标准连接对象
     *
     * 返回一个智能指针管理的标准连接对象，该指针会在销毁时自动调用指定的删除器函数
     *
     * @return std::unique_ptr<Connection, std::function<void(Connection *)>> 包含标准连接对象的智能指针，
     *         使用自定义删除器管理连接生命周期
     */
    std::unique_ptr<Connection, std::function<void(Connection *)>> get_std_conn();

}  // namespace level1

#endif //QUANT1X_LEVEL1_CLIENT_H
