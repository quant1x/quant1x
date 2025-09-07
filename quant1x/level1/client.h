#pragma once
#ifndef QUANT1X_LEVEL1_CLIENT_H
#define QUANT1X_LEVEL1_CLIENT_H 1

#include "quant1x/net/connection_pool.h"
#include "quant1x/std/util.h"
#include "quant1x/std/api.h"
#include "quant1x/encoding/iconv.h"
#include "quant1x/std/buffer.h"

#include "quant1x/exchange/code.h"
#include "quant1x/exchange/session.h"
#include "protocol.h"
#include "encoding.h"
#include "hello1.h"
#include "hello2.h"
#include "heartbeat.h"
#include "xdxr_info.h"
#include "finance_info.h"
#include "security_count.h"
#include "security_list.h"
#include "security_quote.h"
//#include "index_bars.h"
#include "security_bars.h"
#include "transaction_data.h"
#include "transaction_history.h"
#include "block_meta.h"
#include "block_info.h"
//#include "company_category.h"
//#include "company_content.h"
#include "minute_time.h"
#include "config.h"

namespace level1 {

    /// 网络协议
    #pragma pack(push, 1)  // 确保1字节对齐

    #pragma pack(pop)  // 恢复默认对齐方式

    class ProtocolHandler : public NetworkOperationHandler<ProtocolHandler> {
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
