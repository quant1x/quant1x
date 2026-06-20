#pragma once
#ifndef QUANT1X_STD_BUFFER_H
#define QUANT1X_STD_BUFFER_H 1

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

class BinaryStream {
private:
    std::vector<uint8_t> buffer;
    size_t offset;

    template <typename T>
    void push_le(T value) {
        constexpr size_t size = sizeof(T);
        prepare_write(size);
        using Unsigned = std::make_unsigned_t<T>;
        Unsigned u = static_cast<Unsigned>(value);
        for (size_t i = 0; i < size; ++i) {
            buffer[offset + i] = static_cast<uint8_t>(u >> (i * 8));
        }
        offset += size;
    }

    template <typename T>
    T get_le() {
        constexpr size_t size = sizeof(T);
        check_available(size);
        using Unsigned = std::make_unsigned_t<T>;
        Unsigned u = 0;
        for (size_t i = 0; i < size; ++i) {
            u |= static_cast<Unsigned>(buffer[offset + i]) << (i * 8);
        }
        T value;
        std::memcpy(&value, &u, sizeof(T)); // well-defined for trivially-copyable types
        offset += size;
        return value;
    }

    // 对 offset+size 做溢出安全加法并确保容量
    void prepare_write(size_t size) {
        if (offset > SIZE_MAX - size) {
            throw std::overflow_error("Buffer write would overflow size_t");
        }
        size_t required = offset + size;
        if (required > buffer.size()) {
            buffer.resize(required);
        }
    }

    void check_available(size_t required) {
        // Avoid overflow when computing offset + required
        if (required > buffer.size() || offset > buffer.size() - required) {
            throw std::out_of_range("Insufficient data in buffer");
        }
    }

public:
    BinaryStream() : buffer(), offset(0) {}
    explicit BinaryStream(const std::vector<uint8_t>& data) : buffer(data), offset(0) {}
    explicit BinaryStream(const std::vector<char>& data) : buffer(data.begin(), data.end()), offset(0) {}

    template <size_t N>
    explicit BinaryStream(const uint8_t (&data)[N]) :buffer(std::begin(data), std::end(data)), offset(0) {}

    // 禁用拷贝构造和拷贝赋值
    BinaryStream(const BinaryStream&) = delete;
    BinaryStream& operator=(const BinaryStream&) = delete;

    // 析构函数 - 不需要特殊处理, vector会自动清理
    ~BinaryStream() = default;

    // 通用数值类型写入
    template <typename T>
    void push_arithmetic(T value) {
        if constexpr (std::is_floating_point_v<T>) {
            static_assert(sizeof(T) == 4 || sizeof(T) == 8,
                          "Only float (32-bit) and double (64-bit) floating point supported");
            using IntType = std::conditional_t<sizeof(T) == 4, uint32_t, uint64_t>;
            IntType int_val{};
            std::memcpy(&int_val, &value, sizeof(T));
            push_le(int_val);
        } else {
            static_assert(std::is_integral_v<T>, "Only arithmetic types supported");
            push_le(value);
        }
    }

    // 通用数值类型读取
    template <typename T>
    T get_arithmetic() {
        if constexpr (std::is_floating_point_v<T>) {
            static_assert(sizeof(T) == 4 || sizeof(T) == 8,
                          "Only float (32-bit) and double (64-bit) floating point supported");
            using IntType = std::conditional_t<sizeof(T) == 4, uint32_t, uint64_t>;
            IntType int_val = get_le<IntType>();
            T       result{};
            std::memcpy(&result, &int_val, sizeof(T));
            return result;
        } else {
            static_assert(std::is_integral_v<T>, "Only arithmetic types supported");
            return get_le<T>();
        }
    }

    // 基础类型写入(自动推导)
    void push_i8(int8_t value) { push_arithmetic(value); }
    void push_u8(uint8_t value) { push_arithmetic(value); }
    void push_i16(int16_t value) { push_arithmetic(value); }
    void push_u16(uint16_t value) { push_arithmetic(value); }
    void push_i32(int32_t value) { push_arithmetic(value); }
    void push_u32(uint32_t value) { push_arithmetic(value); }
    void push_i64(int64_t value) { push_arithmetic(value); }
    void push_u64(uint64_t value) { push_arithmetic(value); }
    void push_float(float value) { push_arithmetic(value); }
    void push_double(double value) { push_arithmetic(value); }

    // 基础类型读取
    int8_t get_i8() { return get_arithmetic<int8_t>(); }
    uint8_t get_u8() { return get_arithmetic<uint8_t>(); }
    int16_t get_i16() { return get_arithmetic<int16_t>(); }
    uint16_t get_u16() { return get_arithmetic<uint16_t>(); }
    int32_t get_i32() { return get_arithmetic<int32_t>(); }
    uint32_t get_u32() { return get_arithmetic<uint32_t>(); }
    int64_t get_i64() { return get_arithmetic<int64_t>(); }
    uint64_t get_u64() { return get_arithmetic<uint64_t>(); }
    float get_float() { return get_arithmetic<float>(); }
    double get_double() { return get_arithmetic<double>(); }

    // 字节数组写入
    template <size_t N>
    void push_byte_array(const uint8_t (&data)[N]) {
        prepare_write(N);
        std::memcpy(buffer.data() + offset, data, N);
        offset += N;
    }

    template <size_t N>
    void push_byte_array(const std::array<uint8_t, N>& data) {
        push_byte_array(data.data(), N);
    }

    void push_byte_array(const uint8_t* data, size_t n) {
        prepare_write(n);
        std::memcpy(buffer.data() + offset, data, n);
        offset += n;
    }

    // 非字节数组写入(逐元素小端序)
    template <typename T, size_t N>
    void push_array(const T (&data)[N]) {
        for (const auto& elem : data) {
            push_arithmetic(elem);
        }
    }

    template <typename T, size_t N>
    void push_array(const std::array<T, N>& arr) {
        for (const auto& elem : arr) {
            push_arithmetic(elem);
        }
    }

    // 字节数组读取
    template <size_t N>
    void get_byte_array(uint8_t (&output)[N]) {
        check_available(N);
        std::memcpy(output, buffer.data() + offset, N);
        offset += N;
    }

    template <size_t N>
    std::array<uint8_t, N> get_byte_array() {
        std::array<uint8_t, N> arr;
        get_byte_array(arr.data(), N);
        return arr;
    }

    void get_byte_array(uint8_t* output, size_t n) {
        check_available(n);
        std::memcpy(output, buffer.data() + offset, n);
        offset += n;
    }

    // 非字节数组读取(逐元素小端序)
    template <typename T, size_t N>
    void get_array(T (&output)[N]) {
        for (auto& elem : output) {
            elem = get_arithmetic<T>();
        }
    }

    template <typename T, size_t N>
    std::array<T, N> get_array() {
        std::array<T, N> arr;
        for (auto& elem : arr) {
            elem = get_arithmetic<T>();
        }
        return arr;
    }

    // 带长度前缀的字符串处理
    void push_length_prefixed_string(const std::string& str) {
        push_arithmetic<uint32_t>(uint32_t(str.size()));
        push_byte_array(reinterpret_cast<const uint8_t*>(str.data()), str.size());
    }

    std::string get_length_prefixed_string() {
        uint32_t len = get_arithmetic<uint32_t>();
        check_available(len);
        std::string str(reinterpret_cast<const char*>(buffer.data() + offset), len);
        offset += len;
        return str;
    }

    // 原始字符串处理(无长度前缀)
    void push_string(const std::string& str) {
        push_byte_array(reinterpret_cast<const uint8_t*>(str.data()), str.size());
    }

//    std::string get_string(size_t len) {
//        check_available(len);
//        std::string str(reinterpret_cast<const char*>(buffer.data() + offset), len);
//        offset += len;
//        return str;
//    }
    std::string get_string(size_t len) {
        check_available(len); // 确保缓冲区有足够的数据

        const char* start = reinterpret_cast<const char*>(buffer.data() + offset);
        // 使用 std::memchr 查找第一个 '\0'，避免 strnlen (POSIX) 的可移植性问题
        const void* null_pos = std::memchr(start, '\0', len);
        size_t actual_len = null_pos ? static_cast<const char*>(null_pos) - start : len;

        std::string str(start, actual_len); // 截断到第一个 '\0'
        offset += len; // 偏移量始终增加 len

        return str;
    }

    int64_t varint_decode() {
        // Safe, bounds-checked varint decoding that preserves original format
        // Format: first byte: continuation(0x80) | sign(0x40) | 6-bit payload
        // subsequent bytes: continuation(0x80) | 7-bit payload
        size_t pos = offset;
        if (pos >= buffer.size()) {
            throw std::out_of_range("Insufficient data in buffer for varint");
        }

        uint8_t byte = buffer[pos++];
        bool sign = (byte & 0x40) != 0;
        uint64_t data = static_cast<uint64_t>(byte & 0x3F);
        uint32_t shift = 6;

        while (byte & 0x80) {
            if (pos >= buffer.size()) {
                throw std::out_of_range("Insufficient data in buffer during varint decoding");
            }
            byte = buffer[pos++];
            data |= static_cast<uint64_t>(byte & 0x7F) << shift;
            shift += 7;
            if (shift >= 64) {
                // Avoid undefined behavior on excessive shift / overflow
                break;
            }
        }

        // advance the global offset only after successful read
        offset = pos;

        int64_t signed_val;
        if (data > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
            // clamp to int64_t max (preserve behavior but avoid UB)
            signed_val = std::numeric_limits<int64_t>::max();
        } else {
            signed_val = static_cast<int64_t>(data);
        }

        return sign ? -signed_val : signed_val;
    }

    // 工具方法
    [[nodiscard]] size_t position() const { return offset; }

    // 移动光标到指定偏移
    void seek(size_t new_offset) {
        assert(new_offset <= buffer.size() && "seek beyond buffer size");
        offset = new_offset;
    }
    // 在当前偏移的基础上跳过偏移量
    void skip(size_t skip_offset) {
        assert(offset + skip_offset >= offset && "skip overflow"); // guard against size_t wrap
        offset += skip_offset;
    }
    [[nodiscard]] const std::vector<uint8_t>& data() const {
        return buffer;
    }

    void clear() {
        buffer.clear();
        offset = 0;
    }
};

#endif //QUANT1X_STD_BUFFER_H
