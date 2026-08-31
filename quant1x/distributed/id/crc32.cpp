// CRC32-IEEE 的实现
#include <quant1x/distributed/id/crc32.h>

namespace quant1x::distributed::id {

uint32_t crc32_ieee(const uint8_t *data, size_t size) noexcept {
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < size; ++i) {
        const uint32_t index = (crc ^ static_cast<uint32_t>(data[i])) & 0xFFu;
        crc = (crc >> 8) ^ CRC32_IEEE_TABLE[index];
    }
    return crc ^ 0xFFFFFFFFu;
}

}  // namespace quant1x::distributed::id
