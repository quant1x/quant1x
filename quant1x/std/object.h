#pragma once
#ifndef QUANT1X_STD_OBJECT_H
#define QUANT1X_STD_OBJECT_H 1

#include <iostream>
#include <filesystem>
#include <memory>
#include <utility>
#include <vector>
#include <cstring>
#include <mutex>
#include <stdexcept>

#include <quant1x/std/mmap.h>

namespace fs = std::filesystem;

// 缓存头结构体
struct CacheHeader {
    uint32_t headerSize;  // 头信息长度, 包括headerSize字段
    uint32_t magic;       // 魔法数
    uint32_t version;     // 版本
    uint32_t dataSize;    // 数据长度
    uint32_t checksum;    // 数据校验和
    uint32_t elementSize; // 元素尺寸
    uint32_t arrayLen;    // 数组有效长度
    uint32_t arrayCap;    // 数组容量

    friend std::ostream &operator<<(std::ostream &os, const CacheHeader &header) {
        os << "MemObject{" << "\n"
           << " headerSize: " << header.headerSize << "\n"
           << "      magic: " << header.magic << "\n"
           << "    version: " << header.version << "\n"
           << "   dataSize: " << header.dataSize << "\n"
           << "   checksum: " << header.checksum << "\n"
           << "elementSize: " << header.elementSize << "\n"
           << "   arrayLen: " << header.arrayLen << "\n"
           << "   arrayCap: " << header.arrayCap << "\n"
           << "}";
        return os;
    }
};

const uint32_t VERSION = 1;
const uint32_t HEADER_SIZE = sizeof(CacheHeader);
const uint32_t MAGIC_NUMBER = 0xCAC1E5A5;
const size_t MAX_FILE_SIZE = 1 << 30;
constexpr float threshold = 0.8f;

template <typename T>
class MemObject {
public:
    explicit MemObject(std::string name, size_t userSize = 0)
            : filename(std::move(name)), userSize(userSize) {
        openCache();
    }

    ~MemObject() {
        close();
    }

    T* toSlice() {
        std::lock_guard lock(mutex_);

        size_t eSize = sizeof(T);
        if (eSize == 0) {
            throw std::invalid_argument("Zero-sized type");
        }
        auto dataStart = reinterpret_cast<uintptr_t>(m_->data + HEADER_SIZE);
        if (dataStart % alignof(T) != 0) {
            throw std::runtime_error("Memory address not aligned");
        }

        size_t usedElements = header->dataSize / eSize;
        header->arrayCap = usedElements;
        return reinterpret_cast<T*>(m_->data + HEADER_SIZE);
    }

    size_t Add(size_t delta) {
        std::unique_lock lock(mutex_);

        size_t required = header->arrayLen + delta;
//        if (required > size_t(header->arrayCap * threshold)) {
//            expand(required);
//        }

        if (required > header->arrayCap) {
            expand(required);
        }

        header->arrayLen += delta;
        return header->arrayLen - 1;
    }

    CacheHeader* get_header() {
        return header;
    }

    // 重新映射
    void remap() {
        mmap_reopen(m_);
    }

private:
    std::string filename;
    size_t userSize = 0;
    mmap_t *m_ = nullptr;
    CacheHeader* header = nullptr;
    std::mutex mutex_;

    void openCache() {
        size_t elementSize = sizeof(T);
        // 获取当前文件大小
        size_t currentSize = fs::file_size(filename);
        if(currentSize <= HEADER_SIZE) {
            currentSize = HEADER_SIZE;
        }
        auto currentLen = (currentSize - HEADER_SIZE) / elementSize;
        if (currentLen <= 0) {
            userSize = 0;
        } else {
            userSize = currentLen * elementSize;
        }

        size_t totalSize = HEADER_SIZE + userSize;
        if (totalSize > MAX_FILE_SIZE || userSize == 0) {
            throw std::invalid_argument("Invalid size");
        }

        m_ = mmap_open(filename.c_str(), 0, totalSize);

        header = reinterpret_cast<CacheHeader*>(m_->data);
        initHeader();
    }

    void initHeader() {
        if (header->magic == 0) {
            header->magic = MAGIC_NUMBER;
            header->version = VERSION;
            header->dataSize = 0;
            header->checksum = 0;
        } else if (header->magic != MAGIC_NUMBER) {
            throw std::runtime_error("Invalid cache file");
        }
        //verifyData();
        if (header->headerSize + header->dataSize != m_->size) {
            header->headerSize = HEADER_SIZE;
            header->dataSize = m_->size - header->headerSize;
        }
        if (header->elementSize == 0) {
            header->elementSize = sizeof(T);
        }
        auto cap = header->dataSize / header->elementSize;
        if (header->arrayCap != cap) {
            header->arrayCap = cap;
        }
        //updateChecksum();
    }

    void updateChecksum() {
        uint32_t checksum = crc32(0, m_->data, HEADER_SIZE);
        header->checksum = checksum;
    }

    void verifyData() {
        uint32_t checksum = crc32(0, m_->data, HEADER_SIZE);
        if (checksum != header->checksum) {
            throw std::runtime_error("Data checksum mismatch");
        }
    }

    void close() {
        mmap_close(&m_);
    }

    static uint32_t crc32(uint32_t crc, const uint8_t* buf, size_t len) {
        for (size_t i = 0; i < len; ++i) {
            crc ^= buf[i];
            for (int j = 0; j < 8; ++j) {
                crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
            }
        }
        return crc;
    }

    void expand(size_t required) {
        size_t new_cap = header->arrayCap;
        while (new_cap <= required) {
            new_cap *= 2;
        }

        size_t new_user_size = new_cap * sizeof(T);
        size_t new_total_size = HEADER_SIZE + new_user_size;

        // 备份头信息
        CacheHeader old_header = *header;

        // 解除映射并扩展文件
        mmap_resize(m_, new_total_size);

        header = reinterpret_cast<CacheHeader*>(m_->data);
        *header = old_header;
        userSize = new_user_size;
        header->arrayCap = new_cap;
        initHeader();
    }
};


#endif //QUANT1X_STD_OBJECT_H
