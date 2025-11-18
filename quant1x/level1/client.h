#pragma once
#ifndef QUANT1X_LEVEL1_CLIENT_H
#define QUANT1X_LEVEL1_CLIENT_H 1

#include <quant1x/net/connection_pool.h>
#include <quant1x/std/util.h>
#include <quant1x/std/api.h>
#include <quant1x/encoding/charsets.h>
#include <quant1x/std/buffer.h>

#include <quant1x/exchange/code.h>
#include <quant1x/exchange/session.h>
#include <quant1x/level1/protocol.h>
#include <quant1x/level1/helpers.h>
#include <quant1x/level1/hello1.h>
#include <quant1x/level1/hello2.h>
#include <quant1x/level1/heartbeat.h>
#include <quant1x/level1/xdxr_info.h>
#include <quant1x/level1/finance_info.h>
#include <quant1x/level1/security_count.h>
#include <quant1x/level1/security_list.h>
#include <quant1x/level1/security_quote.h>
//#include <quant1x/level1/index_bars.h>
#include <quant1x/level1/security_bars.h>
#include <quant1x/level1/transaction_data.h>
#include <quant1x/level1/transaction_history.h>
#include <quant1x/level1/block_meta.h>
#include <quant1x/level1/block_info.h>
//#include <quant1x/level1/company_category.h>
//#include <quant1x/level1/company_content.h>
#include <quant1x/level1/minute_time.h>
#include <quant1x/level1/config.h>

namespace level1 {

    /// 网络协议
    #pragma pack(push, 1)  // 确保1字节对齐

    #pragma pack(pop)  // 恢复默认对齐方式

    class StandardProtocolHandler : public NetworkOperationHandler<StandardProtocolHandler> {
    public:
        bool handshakeImpl(asio::ip::tcp::socket &socket) {
            try {
                // 第一次协议握手
                Hello1Request reqHello1;
                Hello1Response respHello1;
                process(socket, reqHello1, respHello1);
                // 第二次协议握手
                Hello2Request reqHello2;
                Hello2Response respHello2;
                process(socket, reqHello2, respHello2);
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
                HeartbeatRequest req;
                HeartbeatResponse resp;
                process(socket, req, resp);
                return true;
            } catch (...) {
                return false;
            }
        }
    };

    std::unique_ptr<Connection, std::function<void(Connection*)>> client();

} // namespace level1

#endif //QUANT1X_LEVEL1_CLIENT_H
