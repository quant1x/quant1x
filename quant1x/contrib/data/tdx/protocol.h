#pragma once
#ifndef QUANT1X_CONTRIB_DATA_TDX_PROTOCOL_H
#define QUANT1X_CONTRIB_DATA_TDX_PROTOCOL_H 1

#include <quant1x/encoding/charsets.h>
#include <quant1x/contrib/data/tdx/helpers.h>
#include <quant1x/io/connection_pool.h>
#include <quant1x/std/api.h>
#include <quant1x/std/buffer.h>
#include <quant1x/std/except.h>
#include <quant1x/std/util.h>

namespace quant1x::contrib::data::tdx {

    // 标准行情命令字 (使用强类型枚举)
    enum StdCommand : u16 {
        HEARTBEAT                = 0x0004,  // 心跳维持
        LOGIN1                   = 0x000d,  // 第一次登录
        LOGIN2                   = 0x0fdb,  // 第二次登录
        XDXR_INFO                = 0x000f,  // 除权除息信息
        FINANCE_INFO             = 0x0010,  // 财务信息
        PING                     = 0x0015,  // 测试连接
        COMPANY_CATEGORY         = 0x02cf,  // 公司信息分类
        COMPANY_CONTENT          = 0x02d0,  // 公司信息描述
        SECURITY_COUNT           = 0x044e,  // 证券数量
        SECURITY_LIST            = 0x044d,  // 证券列表
        OLD_SECURITY_LIST        = 0x0450,  // 证券列表, 已废弃, 缺少北交所证券代码列表
        INDEX_BARS               = 0x052d,  // 指数K线
        SECURITY_BARS            = 0x052d,  // 股票K线
        SECURITY_QUOTES_OLD      = 0x053e,  // 旧版行情信息
        SECURITY_QUOTES_NEW      = 0x054c,  // 新版行情信息
        MINUTE_TIME_DATA         = 0x051d,  // 分时数据
        BLOCK_META               = 0x02c5,  // 板块文件信息
        BLOCK_DATA               = 0x06b9,  // 板块文件数据
        TRANSACTION_DATA         = 0x0fc5,  // 分笔成交信息
        HISTORY_MINUTE_DATA      = 0x0fb4,  // 历史分时信息
        HISTORY_TRANSACTION_DATA = 0x0fb5   // 历史分笔成交信息
    };

    // 标准行情命令字转字符串
    inline const char *command_to_string(const StdCommand &cmd) noexcept {
        switch (cmd) {
            case StdCommand::HEARTBEAT:
                return "L1:HEARTBEAT";
            case StdCommand::LOGIN1:
                return "L1:LOGIN1";
            case StdCommand::LOGIN2:
                return "L1:LOGIN2";
            case StdCommand::XDXR_INFO:
                return "L1:XDXR_INFO";
            case StdCommand::FINANCE_INFO:
                return "L1:FINANCE_INFO";
            case StdCommand::PING:
                return "L1:PING";
            case StdCommand::COMPANY_CATEGORY:
                return "L1:COMPANY_CATEGORY";
            case StdCommand::COMPANY_CONTENT:
                return "L1:COMPANY_CONTENT";
            case StdCommand::SECURITY_COUNT:
                return "L1:SECURITY_COUNT";
            case StdCommand::SECURITY_LIST:
                return "L1:SECURITY_LIST";
                // case StdCommand::INDEX_BARS:
                //   return "L1:INDEX_BARS";       // 注意: 与SECURITY_BARS值相同
            case StdCommand::SECURITY_BARS:
                return "L1:SECURITY_BARS";  // 需确认协议设计是否冲突
            case StdCommand::SECURITY_QUOTES_OLD:
                return "L1:SECURITY_QUOTES_OLD";
            case StdCommand::SECURITY_QUOTES_NEW:
                return "L1:SECURITY_QUOTES_NEW";
            case StdCommand::MINUTE_TIME_DATA:
                return "L1:MINUTE_TIME_DATA";
            case StdCommand::BLOCK_META:
                return "L1:BLOCK_META";
            case StdCommand::BLOCK_DATA:
                return "L1:BLOCK_DATA";
            case StdCommand::TRANSACTION_DATA:
                return "L1:TRANSACTION_DATA";
            case StdCommand::HISTORY_MINUTE_DATA:
                return "L1:HISTORY_MINUTE_DATA";
            case StdCommand::HISTORY_TRANSACTION_DATA:
                return "L1:HISTORY_TRANSACTION_DATA";
            default:
                return "L1:UNKNOWN_CMD";
        }
    }

    inline const char *command_to_string(u16 cmd) noexcept {
        return command_to_string(StdCommand(cmd));
    }

    // 生成序列号
    inline uint32_t get_sequence_id() noexcept {
        static std::atomic<uint32_t> _seqId{0};
        return ++_seqId;  // 前置递增保证原子性
    }

    // 压缩标志位处理 (使用constexpr)
    namespace ZlibFlag {
        constexpr u8 Zip          = 0x10;                    // zip压缩标志位
        constexpr u8 Uncompressed = 0x0C;                    // 未压缩
        constexpr u8 Zipped       = Zip | Uncompressed;      // 0x1C
    }  // namespace ZlibFlag

    std::vector<uint8_t> unzip(const std::vector<uint8_t> &buf, uint32_t unzip_size);

    template <typename Derived>
    struct header {
        std::string command() { return static_cast<Derived *>(this)->command_impl(); }

        [[nodiscard]] std::string headerString() const { return static_cast<Derived *>(this)->header_string_impl(); }

        friend std::ostream &operator<<(std::ostream &os, const header &obj) {
            os << obj.headerString();
            return os;
        }
    };

    /// 网络协议
#pragma pack(push, 1)  // 确保1字节对齐

    template <typename Derived>
    struct RequestHeader : public header<Derived> {
        u8  frame_type;    // 帧类型标志
        u32 seq_id;        // 请求编号
        u8  packet_ctrl;   // 数据包控制位
        u16 body_wire_len; // 消息体长度1
        u16 body_raw_len;  // 消息体长度2
        u16 cmd_id;        // 命令字

        RequestHeader() : frame_type(0), seq_id(0), packet_ctrl(0), body_wire_len(0), body_raw_len(0), cmd_id(0) {}

        std::string command_impl() { return command_to_string(cmd_id); }

        std::vector<u8> serialize() { return static_cast<Derived *>(this)->serialize_impl(); }

        std::string to_string() { return static_cast<Derived *>(this)->to_string_impl(); }

        std::vector<u8> header_serialize() {
            spdlog::debug("RequestHeader");
            BinaryStream stream;
            stream.push_arithmetic(frame_type);
            stream.push_arithmetic(seq_id);
            stream.push_arithmetic(packet_ctrl);
            stream.push_arithmetic(body_wire_len);
            stream.push_arithmetic(body_raw_len);
            stream.push_arithmetic(cmd_id);
            return stream.data();
        }

        [[nodiscard]] std::string header_string_impl() const {
            return fmt::format(
                "RequestHeader{{frame_type:{}, seq_id:{}, packet_ctrl:{}, body_wire_len:{}, body_raw_len:{}, cmd_id:{:#06x}}}",
                frame_type,
                seq_id,
                packet_ctrl,
                body_wire_len,
                body_raw_len,
                cmd_id);
        }
    };

    template <typename Derived>
    struct ResponseHeader : public header<Derived> {
        u32 magic_number;            // 对应Go的uint32
        u8  frame_type;    // frame_type
        u32 seq_id;        // 请求编号
        u8  packet_ctrl;   // 数据包控制位
        u16 cmd_id;        // 命令字
        u16 body_wire_len; // 长度
        u16 body_raw_len;  // 未压缩长度

        ResponseHeader() {
            magic_number  = 0;
            frame_type    = 0;
            seq_id        = 0;
            packet_ctrl   = 0;
            cmd_id        = 0;
            body_wire_len = 0;
            body_raw_len  = 0;
        }

        std::string command_impl() { return command_to_string(cmd_id); }

        void deserialize(const std::vector<u8> &data) { static_cast<Derived *>(this)->deserialize_impl(data); }

        std::string to_string() { return static_cast<Derived *>(this)->to_string_impl(); }

        void header_deserialize(const std::vector<u8> &data) {
            BinaryStream stream(data);
            magic_number  = stream.get_u32();
            frame_type    = stream.get_u8();
            seq_id        = stream.get_u32();
            packet_ctrl   = stream.get_u8();
            cmd_id        = stream.get_u16();
            body_wire_len = stream.get_u16();
            body_raw_len  = stream.get_u16();
        }

        [[nodiscard]] std::string header_string_impl() const {
            return fmt::format(
                "ResponseHeader{{magic_number:{}, frame_type:{} seq_id:{}, packet_ctrl:{}, cmd_id:{}, body_wire_len:{}, body_raw_len:{}}}",
                magic_number,
                frame_type,
                seq_id,
                packet_ctrl,
                command_to_string(cmd_id),
                body_wire_len,
                body_raw_len);
        }
    };
#pragma pack(pop)  // 恢复默认对齐方式

    constexpr auto request_header_length  = 0x0c;
    constexpr auto response_header_length = 0x10;

    /**
     * BaseMessage — 消息基类 (对齐 Python protocol.BaseMessage)
     *
     * 用于处理消息头和消息体的解析和序列化. 
     * Python 参考: quant1x/contrib/data/tdx/protocol.py BaseMessage
     */
    template <typename Derived>
    struct BaseMessage {
        RequestHeader<Derived> request_header;
        ResponseHeader<Derived> response_header;

        BaseMessage() : request_header(), response_header() {}

        /// 序列化请求体 (子类实现)
        std::vector<u8> serialize_request_body() { return static_cast<Derived *>(this)->serialize_request_body_impl(); }

        /// 序列化整个请求 = 消息头 + 消息体
        std::vector<u8> serialize_request() {
            auto body = serialize_request_body();
            request_header.body_wire_len = u16(2 + body.size());
            request_header.body_raw_len = u16(2 + body.size());
            auto buf = request_header.header_serialize();
            buf.insert(buf.end(), body.begin(), body.end());
            return buf;
        }

        /// 反序列化响应头
        void deserialize_response_header(const std::vector<u8> &data) {
            response_header.header_deserialize(data);
        }

        /// 反序列化响应体 (子类实现)
        void deserialize_response_body(const std::vector<u8> &data) {
            static_cast<Derived *>(this)->deserialize_response_body_impl(data);
        }

        /// 获取命令字符串
        std::string command() { return request_header.command_impl(); }

        /// 获取请求字符串表示
        std::string request_string() { return request_header.header_string_impl(); }

        /// 获取响应字符串表示
        std::string response_string() { return response_header.header_string_impl(); }

        /// 完整 to_string
        std::string to_string() { return static_cast<Derived *>(this)->to_string_impl(); }
    };

    // 基于 BaseMessage 的 process 函数 (对齐 Python process_level1_new)
    template <typename MessageType>
    quant1x::error process_message(asio::ip::tcp::socket &socket, BaseMessage<MessageType> &msg) {
        std::string cmd     = msg.command();
        auto        req_buf = msg.serialize_request();
        spdlog::debug("[{}]Send buffer: {}", cmd, strings::bytesToHex(req_buf));
        spdlog::debug("[{}]Send request: {}", cmd, msg.request_header.header_string_impl());
        asio::error_code ec;
        size_t n = asio::write(socket, asio::buffer(req_buf.data(), req_buf.size()), ec);
        spdlog::debug("[{}]Send request: {} bytes.", cmd, n);
        if (ec) {
            return quant1x::make_error_code(ec.value(), ec.message());
        }
        // 读取响应的消息头
        std::vector<u8> hdr_response_buf(response_header_length);
        size_t hdr_response_length = asio::read(socket, asio::buffer(hdr_response_buf), ec);
        if (ec) {
            return quant1x::make_error_code(ec.value(), ec.message());
        }
        hdr_response_buf.resize(hdr_response_length);
        msg.deserialize_response_header(hdr_response_buf);
        if (msg.response_header.body_wire_len == 0) {
            return quant1x::make_error_code(0, "success");
        }
        spdlog::debug("[{}]Recv response head: {}", cmd, msg.response_header.header_string_impl());
        std::vector<u8> body_buffer(msg.response_header.body_wire_len);
        size_t body_received = asio::read(socket, asio::buffer(body_buffer, body_buffer.size()), ec);
        if (ec) {
            return quant1x::make_error_code(ec.value(), ec.message());
        }
        body_buffer.resize(body_received);
        if (msg.response_header.body_wire_len != msg.response_header.body_raw_len) {
            std::vector<u8> un = unzip(body_buffer, msg.response_header.body_raw_len);
            body_buffer        = un;
        }
        msg.deserialize_response_body(body_buffer);
        spdlog::debug("[{}]Recv response body: {}", cmd, msg.to_string());
        return quant1x::make_error_code(0, "success");
    }
}  // namespace quant1x::contrib::data::tdx

#endif  // QUANT1X_CONTRIB_DATA_TDX_PROTOCOL_H
