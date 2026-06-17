#include <quant1x/contrib/data/tdx/protocol.h>

namespace level1 {

    std::vector<uint8_t> unzip(const std::vector<uint8_t>& buf, uint32_t unzip_size) {
        if (buf.empty()) {
            return {};
        }

        std::vector<uint8_t> out(unzip_size);
        uLongf destLen = static_cast<uLongf>(unzip_size);

        // 检查 unzip_size 是否溢出 uLongf(通常不会, 但需确保)
        if (destLen != unzip_size) {
            throw std::runtime_error("unzip_size exceeds maximum supported by zlib");
        }

        int ret = uncompress(
            out.data(), &destLen,
            buf.data(), static_cast<uLong>(buf.size())
        );

        // 处理错误码
        switch (ret) {
            case Z_OK:
                if (destLen != unzip_size) {
                    throw std::runtime_error("Decompressed size does not match expected size");
                }
                return out;
            case Z_MEM_ERROR:
                throw std::runtime_error("Zlib memory error");
            case Z_BUF_ERROR:
                throw std::runtime_error("Output buffer too small or input data corrupted");
            case Z_DATA_ERROR:
                throw std::runtime_error("Input data corrupted");
            default:
                throw std::runtime_error("Unknown zlib error: " + std::to_string(ret));
        }
    }

} // namespace level1