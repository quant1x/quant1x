#pragma once
#ifndef QUANT1X_LEVEL1_PROTOCOL_H
#define QUANT1X_LEVEL1_PROTOCOL_H 1

#include <quant1x/encoding/charsets.h>
#include <quant1x/contrib/data/tdx/level1/helpers.h>
#include <quant1x/io/connection_pool.h>
#include <quant1x/std/api.h>
#include <quant1x/std/buffer.h>
#include <quant1x/std/except.h>
#include <quant1x/std/util.h>

namespace level1 {

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
    inline const char *commandToString(const StdCommand &cmd) noexcept {
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

    inline const char *commandToString(u16 cmd) noexcept {
        return commandToString(StdCommand(cmd));
    }

    // 生成序列号
    inline uint32_t SequenceId() noexcept {
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
        std::string command() { return static_cast<Derived *>(this)->commandImpl(); }

        [[nodiscard]] std::string headerString() const { return static_cast<Derived *>(this)->headerStringImpl(); }

        friend std::ostream &operator<<(std::ostream &os, const header &obj) {
            os << obj.headerString();
            return os;
        }
    };

    /// 网络协议
#pragma pack(push, 1)  // 确保1字节对齐

    template <typename Derived>
    struct RequestHeader : public header<Derived> {
        u8  ZipFlag;     // 压缩标志
        u32 SeqID;       // 请求编号
        u8  PacketType;  // 包类型
        u16 PkgLen1;     // 消息体长度1
        u16 PkgLen2;     // 消息体长度2
        u16 Method;      // 请求方法

        RequestHeader() : ZipFlag(0), SeqID(0), PacketType(0), PkgLen1(0), PkgLen2(0), Method(0) {}

        std::string commandImpl() { return commandToString(Method); }

        std::vector<u8> serialize() { return static_cast<Derived *>(this)->serializeImpl(); }

        std::string toString() { return static_cast<Derived *>(this)->toStringImpl(); }

        std::vector<u8> headerSerialize() {
            spdlog::debug("RequestHeader");
            BinaryStream stream;
            stream.push_arithmetic(ZipFlag);
            stream.push_arithmetic(SeqID);
            stream.push_arithmetic(PacketType);
            stream.push_arithmetic(PkgLen1);
            stream.push_arithmetic(PkgLen2);
            stream.push_arithmetic(Method);
            return stream.data();
        }

        [[nodiscard]] std::string headerStringImpl() const {
            return fmt::format(
                "RequestHeader{{ZipFlag:{}, SeqID:{}, PacketType:{}, PkgLen1:{}, PkgLen2:{}, Method:{:#06x}}}",
                ZipFlag,
                SeqID,
                PacketType,
                PkgLen1,
                PkgLen2,
                Method);
        }
    };

    template <typename Derived>
    struct ResponseHeader : public header<Derived> {
        u32 I1;         // 对应Go的uint32
        u8  ZipFlag;    // ZipFlag
        u32 SeqID;      // 请求编号
        u8  I2;         // 对应Go的uint8
        u16 Method;     // 命令字
        u16 ZipSize;    // 长度
        u16 UnZipSize;  // 未压缩长度

        ResponseHeader() {
            I1        = 0;
            ZipFlag   = 0;
            SeqID     = 0;
            I2        = 0;
            Method    = 0;
            ZipSize   = 0;
            UnZipSize = 0;
        }

        std::string commandImpl() { return commandToString(Method); }

        void deserialize(const std::vector<u8> &data) { static_cast<Derived *>(this)->deserializeImpl(data); }

        std::string toString() { return static_cast<Derived *>(this)->toStringImpl(); }

        void headerDeserialize(const std::vector<u8> &data) {
            BinaryStream stream(data);
            I1        = stream.get_u32();
            ZipFlag   = stream.get_u8();
            SeqID     = stream.get_u32();
            I2        = stream.get_u8();
            Method    = stream.get_u16();
            ZipSize   = stream.get_u16();
            UnZipSize = stream.get_u16();
        }

        [[nodiscard]] std::string headerStringImpl() const {
            return fmt::format(
                "ResponseHeader{{I1:{}, ZipFlag:{} SeqID:{}, I2:{}, Method:{}, ZipSize:{}, UnZipSize:{}}}",
                I1,
                ZipFlag,
                SeqID,
                I2,
                commandToString(Method),
                ZipSize,
                UnZipSize);
        }
    };
#pragma pack(pop)  // 恢复默认对齐方式

    constexpr auto request_header_length  = 0x0c;
    constexpr auto response_header_length = 0x10;

    // 模板化的 process 函数
    template <typename RequestType, typename ResponseType>
    quant1x::error process(asio::ip::tcp::socket &socket, RequestType &request, ResponseType &response) {
        std::string cmd     = request.command();
        auto        req_buf = request.serialize();
        spdlog::debug("[{}]Send buffer: {}", cmd, strings::bytesToHex(req_buf));
        spdlog::debug("[{}]Send request: {}", cmd, request.toString());
        asio::error_code ec;
        size_t n = asio::write(socket, asio::buffer(req_buf.data(), req_buf.size()), ec);
        spdlog::debug("[{}]Send request: {} bytes.", cmd, n);
        if (ec) {
            return quant1x::make_error_code(ec.value(), ec.message());
        }
        spdlog::debug("[{}]Recv -1", cmd);
        // 读取响应的消息头
        std::vector<u8> hdr_response_buf(response_header_length);
        spdlog::debug("[{}]Recv -2", cmd);
        size_t hdr_response_length = asio::read(socket, asio::buffer(hdr_response_buf), ec);
        spdlog::debug("[{}]Recv -3", cmd);
        if (ec) {
            return quant1x::make_error_code(ec.value(), ec.message());
        }
        hdr_response_buf.resize(hdr_response_length);
        spdlog::debug("[{}]Recv -4", cmd);
        spdlog::debug("[{}]Recv buffer: {}", cmd, strings::bytesToHex(hdr_response_buf));
        // auto pkg_response_hdr = cista::unchecked_deserialize<tdx::StdResponseHeader>(hdr_response_buf);
        // StdResponseHeader pkg_response_hdr;
        // ResponseHeader<ResponseType> &pkg_response_hdr = dynamic_cast<ResponseHeader<ResponseType>&>(response);
        // pkg_response_hdr.ResponseHeader<ResponseType>::headerDeserialize(hdr_response_buf);
        // response.ResponseHeader<ResponseType>::headerDeserialize(hdr_response_buf);
        response.headerDeserialize(hdr_response_buf);
        // 处理接收到的数据
        spdlog::debug("[{}]Recv response head: {}", cmd, response.headerStringImpl());
        if (response.ZipSize == 0) {
            return quant1x::make_error_code(0, "success");
        }
        std::vector<u8> body_buffer(response.ZipSize);
        spdlog::debug("[{}]Recv response body_buffer.size() = {}", cmd, body_buffer.size());
        size_t body_received = asio::read(socket, asio::buffer(body_buffer, body_buffer.size()), ec);
        if (ec) {
            return quant1x::make_error_code(ec.value(), ec.message());
        }
        body_buffer.resize(body_received);
        if (response.ZipSize != response.UnZipSize) {
            std::vector<u8> un = unzip(body_buffer, response.UnZipSize);
            body_buffer        = un;
        }
        spdlog::debug("[{}]Recv response buff: {}", cmd, strings::bytesToHex(body_buffer));
        response.deserialize(body_buffer);
        spdlog::debug("[{}]Recv response body: {}", cmd, response.toString());
        return quant1x::make_error_code(0, "success");
    }
}  // namespace level1
#endif  // QUANT1X_LEVEL1_PROTOCOL_H
