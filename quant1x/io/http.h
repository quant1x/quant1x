#pragma once
#ifndef QUANT1X_IO_HTTP_H
#define QUANT1X_IO_HTTP_H 1

#include <string>
#include <cstdint>

namespace io {
    // If-Modified-Since
    std::tuple<std::string, int64_t> request(const std::string &url, int64_t fileLastModified = 0);
}

#endif //QUANT1X_IO_HTTP_H
