#include <quant1x/encoding/charsets.h>
#include <iconv.h>
#include <vector>
#include <stdexcept>
#include <thread>
#include <cstring>
#include <cerrno>

/// 编码
namespace charsets {

    /// 匿名命名空间, 禁止访问内部功能
    namespace {
        class IconvInstance {
        public:
            IconvInstance(const char* to, const char* from) {
                cd_ = iconv_open(to, from);
                if (cd_ == (iconv_t)-1) {
                    throw std::runtime_error("iconv_open failed");
                }
            }

            ~IconvInstance() {
                iconv_close(cd_);
            }

            // 按照 NUL (0x00) 截断 buf：返回有效长度（更高效地使用 memchr）
            static size_t truncate_at_null(const char *buf, size_t length) noexcept {
                if (buf == nullptr || length == 0) return 0;
                // memchr 在大块内查找 NUL，通常比逐字节循环更快，且由 libc 优化
                const void* p = std::memchr(buf, '\0', length);
                if (p == nullptr) return length;
                return static_cast<const char*>(p) - buf;
            }

            std::string convert(const std::string& input) {
                // 重置转换器状态（关键步骤：清空内部状态）
                iconv(cd_, nullptr, nullptr, nullptr, nullptr);

                size_t in_bytes_left = input.size();
                // iconv 会修改指针值，但不会修改源数据。使用 const_cast 保持兼容旧实现。
                char* in_buf = const_cast<char*>(input.data());
                // 截断输入长度到首个 NUL（若有），使用新的 truncate_at_null 返回长度
                in_bytes_left = truncate_at_null(in_buf, in_bytes_left);

                // 确保输出缓冲区有非零的初始大小，避免在空输入时将空指针传给 iconv
                size_t initial_out = input.size() * 4;
                const size_t kMinOutBuf = 64;
                if (initial_out < kMinOutBuf) initial_out = kMinOutBuf;
                std::vector<char> out_buf(initial_out, 0);
                size_t out_buf_size = out_buf.size();
                char* out_ptr = out_buf.data();
                size_t out_bytes_left = out_buf_size;

                // 转换输入
                while (in_bytes_left > 0) {
                    errno = 0;
                    size_t ret = iconv(cd_, &in_buf, &in_bytes_left, &out_ptr, &out_bytes_left);
                    if (ret != (size_t)-1) {
                        // 成功/部分成功，继续循环直到输入耗尽
                        continue;
                    }

                    // 发生错误
                    if (errno == E2BIG) {
                        // 输出缓冲区不足，扩展并继续
                        size_t used = out_ptr - out_buf.data();
                        out_buf_size = out_buf_size * 2 + 16;
                        out_buf.resize(out_buf_size);
                        out_ptr = out_buf.data() + used;
                        out_bytes_left = out_buf_size - used;
                        continue;
                    } else if (errno == EILSEQ) {
                        // 非法的多字节序列：跳过一个字节，写入替代字符，并继续
                        if (out_bytes_left == 0) {
                            size_t used = out_ptr - out_buf.data();
                            out_buf_size = out_buf_size * 2 + 16;
                            out_buf.resize(out_buf_size);
                            out_ptr = out_buf.data() + used;
                            out_bytes_left = out_buf_size - used;
                        }
                        // 插入替代字符（'?'）
                        *out_ptr++ = '?';
                        --out_bytes_left;
                        // 跳过一个输入字节，继续
                        ++in_buf;
                        --in_bytes_left;
                        continue;
                    } else if (errno == EINVAL) {
                        // 不完整的多字节序列（通常在输入末尾）：插入替代并结束
                        if (out_bytes_left == 0) {
                            size_t used = out_ptr - out_buf.data();
                            out_buf_size = out_buf_size * 2 + 16;
                            out_buf.resize(out_buf_size);
                            out_ptr = out_buf.data() + used;
                            out_bytes_left = out_buf_size - used;
                        }
                        *out_ptr++ = '?';
                        --out_bytes_left;
                        break;
                    } else {
                        // 未知错误：抛出异常以便上层可见（比静默返回源更安全）
                        throw std::runtime_error(std::string("iconv failed, errno=") + std::to_string(errno));
                    }
                }

                // 通过向 iconv 传入 inbuf == NULL 来刷新（flush）任何移位/状态序列的输出
                for (;;) {
                    errno = 0;
                    size_t ret = iconv(cd_, nullptr, nullptr, &out_ptr, &out_bytes_left);
                    if (ret != (size_t)-1) break;
                    if (errno == E2BIG) {
                        size_t used = out_ptr - out_buf.data();
                        out_buf_size = out_buf_size * 2 + 16;
                        out_buf.resize(out_buf_size);
                        out_ptr = out_buf.data() + used;
                        out_bytes_left = out_buf_size - used;
                        continue;
                    }
                    // 其它错误在 flush 阶段通常可忽略，使用已转换的内容
                    break;
                }

                auto size = static_cast<std::string::size_type>(out_ptr - out_buf.data());
                return {out_buf.data(), size};
            }

        private:
            iconv_t cd_;
        };
    }

    std::string utf8_to_gbk(const std::string& utf8_str) {
        thread_local static IconvInstance converter("GB18030//IGNORE", "UTF-8//IGNORE");
        return converter.convert(utf8_str);
    }

    std::string gbk_to_utf8(const std::string& gbk_str) {
        thread_local static IconvInstance converter("UTF-8//IGNORE", "GB18030//IGNORE");
        return converter.convert(gbk_str);
    }
} // namespace charsets