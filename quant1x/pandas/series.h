#pragma once
#ifndef QUANT1X_PANDAS_SERIES_H
#define QUANT1X_PANDAS_SERIES_H 1

#include <quant1x/std/simd.h>
#include "ewm.h"

// 前置声明EWM（避免循环引用）
//struct EW;
//class ExponentialMovingWindow;

namespace ta {

    template <typename T>
    class Series {
    public:
        //=== 构造函数组 ===//
        explicit Series(std::span<T> data) : data_(data) {}
        explicit Series(std::vector<T>& vec)
            : data_(vec.data(), vec.size()) {}

        // 手动实现拷贝构造函数
        Series(const Series& other) : data_(other.data_) {}

        //=== 禁止拷贝（避免意外的数据共享）===//
        //Series(const Series&) = delete;
        //Series& operator=(const Series&) = delete;

        //== 允许移动 ===//
        Series<T>(Series<T>&&) = default;
        Series<T>& operator=(Series<T>&&) = default;

        //=== 数据访问 ===//
        std::span<T> data() const noexcept { return data_; }
        size_t size() const noexcept { return data_.size(); }
        bool empty() const noexcept { return data_.empty(); }

        //=== 移位操作 ===//
        Series<T> shift(int periods) {
            std::vector<T> res(data_.size(), T{});
            if (periods > 0) {
                size_t offset = static_cast<size_t>(periods);
                if (offset < data_.size()) {
                    std::copy(data_.begin(), data_.end() - offset,
                              res.begin() + offset);
                }
            } else if (periods < 0) {
                size_t offset = static_cast<size_t>(-periods);
                if (offset < data_.size()) {
                    std::copy(data_.begin() + offset, data_.end(),
                              res.begin());
                }
            }
            return Series<T>(res);
        }

        Series<T> align(size_t target_size, T fill_value=T{}) const {
            if (data_.size() >= target_size) {
                // 裁剪模式：只保留前 target_size 个元素
                return Series<T>(std::vector<T>(data_.data(), data_.data() + target_size));
            } else {
                // 扩展模式：复制原数据，并用 fill_value 填充剩余部分
                std::vector<T> new_data(data_.begin(), data_.end());
                new_data.insert(new_data.end(), target_size - data_.size(), fill_value);
                return Series<T>(new_data);
            }
        }

        //=== NaN处理 ===//
        template <typename U = T>
        auto fill_nan(U value) const -> std::enable_if_t<std::is_floating_point_v<U>, Series<T>> {
            std::vector<T> res(data_.begin(), data_.end());
            for (auto& v : res) {
                if (std::isnan(v)) v = value;
            }
            return Series<T>(res);
        }

        // 禁止非浮点类型调用
        template <typename U = T>
        auto fill_nan(U) const -> std::enable_if_t<!std::is_floating_point_v<U>, void> {
            static_assert(std::is_floating_point_v<U>,
                          "fill_nan() requires floating-point type");
        }

        void check_size(const Series<T>& other) const {
            if (data_.size() != other.data_.size()) {
                throw std::invalid_argument(
                    "Series size mismatch: " +
                    std::to_string(data_.size()) + " vs " +
                    std::to_string(other.data_.size()));
            }
        }

        // ewm()方法实现移至类定义内
        template <typename EWType>
        ExponentialMovingWindow<T> ewm(EWType&& param) {
            return ExponentialMovingWindow<T>(*this, std::forward<EWType>(param));
        }

    private:
        std::span<T> data_;
    };

    #include <iterator>
    class bool_iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = bool;
        using difference_type = std::ptrdiff_t;
        using pointer = const bool*;
        using reference = const bool&;

        explicit bool_iterator(const uint8_t* ptr) : ptr_(ptr) {}

        bool operator*() const { return *ptr_ != 0; }

        bool_iterator& operator++() { ++ptr_; return *this; }
        bool_iterator operator++(int) { bool_iterator tmp(*this); ++ptr_; return tmp; }

        bool_iterator& operator--() { --ptr_; return *this; }
        bool_iterator operator--(int) { bool_iterator tmp(*this); --ptr_; return tmp; }

        bool_iterator& operator+=(difference_type n) { ptr_ += n; return *this; }
        bool_iterator& operator-=(difference_type n) { ptr_ -= n; return *this; }

        friend bool_iterator operator+(const bool_iterator& it, difference_type n) {
            return bool_iterator(it.ptr_ + n);
        }

        friend bool_iterator operator-(const bool_iterator& it, difference_type n) {
            return bool_iterator(it.ptr_ - n);
        }

        difference_type operator-(const bool_iterator& other) const {
            return ptr_ - other.ptr_;
        }

        bool operator==(const bool_iterator& other) const { return ptr_ == other.ptr_; }
        bool operator!=(const bool_iterator& other) const { return ptr_ != other.ptr_; }

        bool operator<(const bool_iterator& other) const { return ptr_ < other.ptr_; }
        bool operator>(const bool_iterator& other) const { return ptr_ > other.ptr_; }
        bool operator<=(const bool_iterator& other) const { return ptr_ <= other.ptr_; }
        bool operator>=(const bool_iterator& other) const { return ptr_ >= other.ptr_; }

    private:
        const uint8_t* ptr_;
    };

    template <>
    class Series<bool> {
    public:
        explicit Series(const std::vector<bool>& data) {
            data_.reserve(data.size());
            for(const auto & v : data) {
                data_.push_back(v?1:0);
            }
        }
        explicit Series(const std::vector<uint8_t>& data) : data_(data) {}

        //=== 移位操作 ===//
        Series<bool> shift(int periods) {
            std::vector<bool> res(data_.size(), false);
            if (periods > 0) {
                size_t offset = static_cast<size_t>(periods);
                if (offset < data_.size()) {
                    std::copy(data_.begin(), data_.end() - offset,
                              res.begin() + offset);
                }
            } else if (periods < 0) {
                size_t offset = static_cast<size_t>(-periods);
                if (offset < data_.size()) {
                    std::copy(data_.begin() + offset, data_.end(),
                              res.begin());
                }
            }
            return Series<bool>(res);
        }

        bool operator[](size_t idx) const {
            return data_[idx] != 0;
        }


        size_t size() const { return data_.size(); }

        std::span<const bool> data() const {
            temp_bool_.clear();
            temp_bool_.reserve(data_.size());
            for (auto v : data_) {
                temp_bool_.push_back(v != 0 ? 1 : 0); // 显式转换为 uint8_t
            }

            if (temp_bool_.empty()) {
                return {};
            } else {
                return std::span<const bool>(
                    reinterpret_cast<const bool*>(temp_bool_.data()),
                    temp_bool_.size()
                );
            }
        }

        bool_iterator begin() const {
            return bool_iterator(data_.data());
        }

        bool_iterator end() const {
            return bool_iterator(data_.data() + data_.size());
        }

        Series<bool> align(size_t target_size, bool fill_value = false) const {
            if (data_.size() >= target_size) {
                // 裁剪
                std::vector<uint8_t> truncated(data_.begin(), data_.begin() + target_size);
                return Series<bool>(truncated);
            } else {
                // 扩展
                std::vector<uint8_t> expanded = data_;
                expanded.insert(expanded.end(), target_size - data_.size(), fill_value ? 1 : 0);
                return Series<bool>(expanded);
            }
        }

    private:
        std::vector<uint8_t> data_;
        mutable std::vector<uint8_t> temp_bool_;
    };

    // 在 ta 命名空间内声明为非成员函数
    template <typename T>
    Series<bool> operator==(const Series<T>& lhs, const Series<T>& rhs) {
        lhs.check_size(rhs);
        std::vector<bool> res;
        res.reserve(lhs.size());
        for (size_t i = 0; i < lhs.size(); ++i) {
            res.push_back(lhs.data()[i] == rhs.data()[i]);
        }
        return Series<bool>(res);
    }

} // namespace ta

#endif // QUANT1X_PANDAS_SERIES_H