// 64 位可排序分布式 ID 的实现
#include <quant1x/distributed/id/id.h>

namespace quant1x::distributed::id {

/// base64url 字符表 (无填充, 与 Go base64.RawURLEncoding 一致; 末位为字符串结束符)
constexpr char BASE64URL_ALPHABET[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

namespace {

/// 将单个 base64url 字符解码为 6 位值, 非法字符返回 -1
int8_t decode_base64url_char(char c) noexcept {
    if (c >= 'A' && c <= 'Z') {
        return static_cast<int8_t>(c - 'A');
    }
    if (c >= 'a' && c <= 'z') {
        return static_cast<int8_t>(c - 'a' + 26);
    }
    if (c >= '0' && c <= '9') {
        return static_cast<int8_t>(c - '0' + 52);
    }
    if (c == '-') {
        return 62;
    }
    if (c == '_') {
        return 63;
    }
    return -1;
}

}  // namespace

void Id::bytes(uint8_t out[8]) const noexcept {
    out[0] = static_cast<uint8_t>((value_ >> 56) & 0xFFu);
    out[1] = static_cast<uint8_t>((value_ >> 48) & 0xFFu);
    out[2] = static_cast<uint8_t>((value_ >> 40) & 0xFFu);
    out[3] = static_cast<uint8_t>((value_ >> 32) & 0xFFu);
    out[4] = static_cast<uint8_t>((value_ >> 24) & 0xFFu);
    out[5] = static_cast<uint8_t>((value_ >> 16) & 0xFFu);
    out[6] = static_cast<uint8_t>((value_ >> 8) & 0xFFu);
    out[7] = static_cast<uint8_t>(value_ & 0xFFu);
}

Id Id::from_bytes(const uint8_t in[8]) noexcept {
    uint64_t value = 0;
    value |= static_cast<uint64_t>(in[0]) << 56;
    value |= static_cast<uint64_t>(in[1]) << 48;
    value |= static_cast<uint64_t>(in[2]) << 40;
    value |= static_cast<uint64_t>(in[3]) << 32;
    value |= static_cast<uint64_t>(in[4]) << 24;
    value |= static_cast<uint64_t>(in[5]) << 16;
    value |= static_cast<uint64_t>(in[6]) << 8;
    value |= static_cast<uint64_t>(in[7]);
    return Id(value);
}

int64_t Id::physical() const noexcept {
    return static_cast<int64_t>(value_ >> PAYLOAD_BITS);
}

uint32_t Id::node_id(uint8_t worker_bits) const noexcept {
    const unsigned shift = static_cast<unsigned>(PAYLOAD_BITS - worker_bits);
    return static_cast<uint32_t>(value_ >> shift) & ((1u << worker_bits) - 1u);
}

uint32_t Id::seq(uint8_t worker_bits) const noexcept {
    const unsigned shift = static_cast<unsigned>(PAYLOAD_BITS - worker_bits);
    return static_cast<uint32_t>(value_) & ((1u << shift) - 1u);
}

std::string Id::to_string() const {
    uint8_t b[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    bytes(b);
    std::string out;
    out.reserve(11);
    out.push_back(BASE64URL_ALPHABET[(b[0] >> 2) & 0x3Fu]);
    out.push_back(BASE64URL_ALPHABET[((b[0] & 0x03u) << 4) | (b[1] >> 4)]);
    out.push_back(BASE64URL_ALPHABET[((b[1] & 0x0Fu) << 2) | (b[2] >> 6)]);
    out.push_back(BASE64URL_ALPHABET[b[2] & 0x3Fu]);
    out.push_back(BASE64URL_ALPHABET[(b[3] >> 2) & 0x3Fu]);
    out.push_back(BASE64URL_ALPHABET[((b[3] & 0x03u) << 4) | (b[4] >> 4)]);
    out.push_back(BASE64URL_ALPHABET[((b[4] & 0x0Fu) << 2) | (b[5] >> 6)]);
    out.push_back(BASE64URL_ALPHABET[b[5] & 0x3Fu]);
    out.push_back(BASE64URL_ALPHABET[(b[6] >> 2) & 0x3Fu]);
    out.push_back(BASE64URL_ALPHABET[((b[6] & 0x03u) << 4) | (b[7] >> 4)]);
    // 末字符: byte[7] 的低 4 位置于 6 位值的高 4 位, 低 2 位补 0 (标准 RawURLEncoding)
    out.push_back(BASE64URL_ALPHABET[(b[7] & 0x0Fu) << 2]);
    return out;
}

Result<Id> Id::parse(const std::string &text) {
    if (text.size() != 11) {
        return Error::parse_id(text);
    }
    uint8_t v[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    for (size_t i = 0; i < 11; ++i) {
        const int8_t decoded = decode_base64url_char(text[i]);
        if (decoded < 0) {
            return Error::parse_id(text);
        }
        v[i] = static_cast<uint8_t>(decoded);
    }
    // 与 Go/Python 一致: 末字符只取高 4 位 (>> 2), 低 2 位为填充位并忽略
    const uint8_t b[8] = {
        static_cast<uint8_t>((v[0] << 2) | (v[1] >> 4)),
        static_cast<uint8_t>((v[1] << 4) | (v[2] >> 2)),
        static_cast<uint8_t>((v[2] << 6) | v[3]),
        static_cast<uint8_t>((v[4] << 2) | (v[5] >> 4)),
        static_cast<uint8_t>((v[5] << 4) | (v[6] >> 2)),
        static_cast<uint8_t>((v[6] << 6) | v[7]),
        static_cast<uint8_t>((v[8] << 2) | (v[9] >> 4)),
        static_cast<uint8_t>(((v[9] & 0x0Fu) << 4) | (v[10] >> 2)),
    };
    return Id::from_bytes(b);
}

Result<int64_t> Id::check_epoch(int64_t elapsed) {
    if (elapsed < 0 || elapsed >= (static_cast<int64_t>(1) << PHYSICAL_BITS)) {
        return Error::epoch_elapsed_out_of_range(elapsed);
    }
    return elapsed;
}

}  // namespace quant1x::distributed::id
