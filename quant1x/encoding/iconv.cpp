#include <quant1x/encoding/iconv.h>
#include <iconv.h>
#include <vector>
#include <stdexcept>
#include <thread>
#include <cstring>

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

            // 按照0x00截断buf
            static void truncate_at_null(const char * const buf, size_t &length) {
                size_t nullPos = 0;
                while (nullPos < length && buf[nullPos] != '\0') {
                    ++nullPos;
                }
                if (nullPos < length) {
                    length = nullPos;
                }
            }

            std::string convert(const std::string& input) {
                // 重置转换器状态（关键步骤！）
                iconv(cd_, nullptr, nullptr, nullptr, nullptr);

                size_t in_bytes_left = input.size();
                char* in_buf = const_cast<char*>(input.data());
                truncate_at_null(in_buf, in_bytes_left);

                std::vector<char> out_buf(input.size() * 4, 0);
                size_t out_buf_size = out_buf.size();
                char* out_ptr = out_buf.data();
                size_t out_bytes_left = out_buf_size;

                while (in_bytes_left > 0) {
                    size_t ret = iconv(cd_, &in_buf, &in_bytes_left, &out_ptr, &out_bytes_left);
                    if (ret == (size_t)-1) {
                        if (errno == E2BIG) {
                            size_t used = out_ptr - out_buf.data();
                            out_buf_size *= 2;
                            out_buf.resize(out_buf_size);
                            out_ptr = out_buf.data() + used;
                            out_bytes_left = out_buf_size - used;
                        } else {
                            //throw make_system_error();
                            return input;
                        }
                    }
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