#pragma once
#ifndef QUANT1X_TECHNICAL_ANALYSIS_ROLLING_WINDOW_H
#define QUANT1X_TECHNICAL_ANALYSIS_ROLLING_WINDOW_H 1

#include <vector>
#include <stdexcept>

namespace ta {

    template<typename T>
    class RollingWindow {
    public:
        explicit RollingWindow(size_t capacity)
            : buffer_(capacity), head_(0), count_(0) {}

        void push(T value) {
            buffer_[head_] = value;
            head_ = (head_ + 1) % buffer_.size();
            count_ = std::min(count_ + 1, buffer_.size());
        }

        const T* data() const {
            return buffer_.data();
        }

        size_t size() const { return count_; }
        size_t capacity() const { return buffer_.size(); }

        T operator[](size_t index) const {
            if (index >= count_) {
                throw std::out_of_range("Index out of range");
            }
            size_t idx = (head_ + count_ - index - 1) % buffer_.size();
            return buffer_[idx];
        }

    private:
        std::vector<T> buffer_;
        size_t head_;
        size_t count_;
    };

} // namespace ta

#endif //QUANT1X_TECHNICAL_ANALYSIS_ROLLING_WINDOW_H
