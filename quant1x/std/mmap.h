#pragma once
#ifndef QUANT1X_STD_MMAP_H
#define QUANT1X_STD_MMAP_H 1

#include <filesystem>
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#include <shared_mutex>

#else
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include <mimalloc-override.h>

typedef struct file_mmap {
    const char * filename = nullptr;
    size_t offset = 0;
    size_t size = 0;
#ifdef _WIN32
    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    HANDLE mapHandle = nullptr;
    /** The start of the real memory page area (mapped view) */
    void *mv = nullptr;
#else
    int fd = -1;
#endif
    uint8_t *data = nullptr;
} mmap_t;


const int DIR_MODE = 0755;
const int FILE_MODE = 0644;

namespace fs = std::filesystem;

static void mmap_destroy(mmap_t **mm);

void file_open(mmap_t *mm, const char * const filename) {
#ifdef _WIN32
    HANDLE fileHandle = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                                    OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Failed to open file");
    }
    mm->fileHandle = fileHandle;
#else
    int fd = open(mm->filename, O_RDWR | O_CREAT, FILE_MODE);
    if (fd < 0) {
        throw std::runtime_error("Failed to open file");
    }
    mm->fd = fd;
#endif
    mm->filename = filename;
}

void file_truncate(mmap_t *mm, size_t fSize) {
#ifdef _WIN32
    if (SetFilePointer(mm->fileHandle, (LONG)fSize, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER ||
        !SetEndOfFile(mm->fileHandle)) {
        CloseHandle(mm->fileHandle);
        throw std::runtime_error("Failed to truncate file");
    }
#else
    if (ftruncate(mm->fd, fSize) < 0) {
        ::close(mm->fd);
        throw std::runtime_error("Failed to truncate file");
    }
#endif
    mm->size = fSize;
}

void file_mmap(mmap_t *mm, int64_t offset, size_t size) {
#ifdef _WIN32
    /* The size of the CreateFileMapping object is the current size
     * of the size of the mmap object (e.g. file size), not the size
     * of the mapped region!
     */
    HANDLE mapHandle = CreateFileMapping(mm->fileHandle, nullptr, PAGE_READWRITE, 0, 0, nullptr);
    if (!mapHandle || mapHandle == INVALID_HANDLE_VALUE) {
        CloseHandle(mm->fileHandle);
        throw std::runtime_error("Failed to create file mapping");
    }
    static DWORD memblock = 0;
    if (!memblock)
    {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        memblock = si.dwAllocationGranularity;
    }
    /** The physical start, size and offset */
    int64_t pstart;
    size_t psize;
    int64_t  poffset;

    pstart = (offset / memblock) * memblock;
    poffset = offset - pstart;
    psize = (size_t)poffset + size;

    DWORD offlo, offhi;
    offlo = (DWORD)pstart;
    offhi = (DWORD)(pstart >> 32);
    mm->mv = MapViewOfFile(mapHandle, FILE_MAP_ALL_ACCESS, offhi, offlo, psize);
    if (!mm->mv) {
        CloseHandle(mapHandle);
        CloseHandle(mm->fileHandle);
        throw std::runtime_error("Failed to map view of file");
    }
    mm->mapHandle = mapHandle;
    mm->data = (uint8_t*)mm->mv + poffset;
#else
    mm->data = static_cast<uint8_t*>(mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, mm->fd, 0));
    if (mm->data == MAP_FAILED) {
        ::close(mm->fd);
        mm->fd = -1;
        throw std::runtime_error("Failed to mmap file");
    }
#endif
    mm->offset = offset;
    mm->size = size;
}

void mmap_unmap(mmap_t *mm) {
#ifdef _WIN32
    if (mm->mv) {
        UnmapViewOfFile(mm->mv);
        mm->mv = nullptr;
    }
    if (mm->mapHandle) {
        CloseHandle(mm->mapHandle);
        mm->mapHandle = nullptr;
    }
#else
    if (mm->data) {
        munmap(mm->data, mm->size);
        mm->data = nullptr;
    }
#endif
}

void file_close(mmap_t *mm) {
#ifdef _WIN32
    if (mm->fileHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(mm->fileHandle);
        mm->fileHandle = INVALID_HANDLE_VALUE;
    }
#else
    if (mm->fd >= 0) {
        ::close(mm->fd);
        mm->fd = -1;
    }
#endif
}

mmap_t * v1_mmap_open(const char * const filename, size_t offset, size_t size) {
    fs::create_directories(fs::path(filename).parent_path());
    mmap_t m;
#ifdef _WIN32
    HANDLE fileHandle = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                             OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Failed to open file");
    }

    if (SetFilePointer(fileHandle, size, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER ||
        !SetEndOfFile(fileHandle)) {
        CloseHandle(fileHandle);
        throw std::runtime_error("Failed to truncate file");
    }

    HANDLE mapHandle = CreateFileMapping(fileHandle, nullptr, PAGE_READWRITE, 0, size, nullptr);
    if (!mapHandle) {
        CloseHandle(fileHandle);
        throw std::runtime_error("Failed to create file mapping");
    }

    uint8_t *data = static_cast<uint8_t*>(MapViewOfFile(mapHandle, FILE_MAP_ALL_ACCESS, 0, 0, size));
    if (!data) {
        CloseHandle(mapHandle);
        CloseHandle(fileHandle);
        throw std::runtime_error("Failed to map view of file");
    }
    m.fileHandle = fileHandle;
    m.mapHandle = mapHandle;
#else
    int fd = open(filename, O_RDWR | O_CREAT, FILE_MODE);
    if (fd < 0) {
        throw std::runtime_error("Failed to open file");
    }

    if (ftruncate(fd, size) < 0) {
        ::close(fd);
        throw std::runtime_error("Failed to truncate file");
    }

    uint8_t *data = static_cast<uint8_t*>(mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (data == MAP_FAILED) {
        ::close(fd);
        throw std::runtime_error("Failed to mmap file");
    }
    m.fd = fd;
#endif
    m.filename = filename;
    m.data = data;
    m.offset = offset;
    m.size = size;
    mmap_t * mm = (mmap_t *)malloc(sizeof(mmap_t));
    memcpy(mm, &m, sizeof(mmap_t ));
    return mm;
}

mmap_t * mmap_open(const char * const filename, size_t offset, size_t size) {
    fs::create_directories(fs::path(filename).parent_path());
    auto * mm = (mmap_t *)malloc(sizeof(mmap_t));
    try {
        file_open(mm, filename);
        file_truncate(mm, size);
        file_mmap(mm, (int64_t)offset, size);
    } catch(...) {
        mmap_destroy(&mm);
    }
    return mm;
}

void mmap_flush(mmap_t *mm) {
    if (mm->data) {
#ifdef _WIN32
        FlushViewOfFile(mm->data, mm->size);
#else
        msync(mm->data, mm->size, MS_SYNC);
#endif
    }
}


void mmap_destroy(mmap_t **mm) {
    if(*mm == nullptr) {
        return;
    }
    mmap_unmap(*mm);
    file_close(*mm);
    free(*mm);
    *mm = nullptr;
}


void mmap_close(mmap_t **mm) {
    mmap_destroy(mm);
}

void mmap_resize(mmap_t *mm, size_t new_size) {
    if(mm == nullptr) {
        return;
    }
    // 刷新缓存
    mmap_flush(mm);
    // 解除映射
    mmap_unmap(mm);
    file_truncate(mm, new_size);
    file_mmap(mm, (int64_t)mm->offset, new_size);
}

void mmap_reopen(mmap_t * mm) {
    if(mm == nullptr) {
        return;
    }
    // 刷新缓存
    mmap_flush(mm);
    // 解除映射
    mmap_unmap(mm);
    size_t new_size = fs::file_size(mm->filename);
    file_mmap(mm, (int64_t)mm->offset, new_size);
}


#endif //QUANT1X_STD_MMAP_H
