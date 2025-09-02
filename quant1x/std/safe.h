#pragma once
#ifndef QUANT1X_STD_SAFE_H
#define QUANT1X_STD_SAFE_H 1

#include <cerrno>   // C标准库errno
#include <cstdio>   // snprintf
#include <cstring>  // C标准库字符串函数
#include <ctime>
#include <string>

namespace q1x {
    namespace safe {
        // 安全的 localtime 函数，避免线程不安全
        std::tm localtime(std::time_t t) noexcept;

        // 安全的 gmtime 函数，避免线程不安全
        std::tm gmtime(std::time_t t) noexcept;

        inline std::string strerror(int errnum) {
            constexpr size_t buf_size = 256;
            std::string      buf(buf_size, '\0');

#if defined(_WIN32) || defined(_MSC_VER)
            if (strerror_s(&buf[0], buf.size(), errnum) != 0) {
                std::snprintf(&buf[0], buf.size(), "Unknown error %d", errnum);
            }
#elif defined(__APPLE__) || ((_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && !_GNU_SOURCE)
            if (strerror_r(errnum, &buf[0], buf.size()) != 0) {
                std::snprintf(&buf[0], buf.size(), "Unknown error %d", errnum);
            }
#else
            char *msg = strerror_r(errnum, &buf[0], buf.size());
            if (msg != &buf[0]) {
                std::strncpy(&buf[0], msg, buf.size());
                buf[buf.size() - 1] = '\0';
            }
#endif

            buf.resize(std::strlen(buf.c_str()));
            return buf;
        }
    }  // namespace safe

}  // namespace q1x

#endif  // QUANT1X_STD_SAFE_H