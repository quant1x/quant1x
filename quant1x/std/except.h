#pragma once
#ifndef QUANT1X_STD_EXCEPT_H
#define QUANT1X_STD_EXCEPT_H 1

#include <iostream>
#include <exception>
#include <string>
#include <stdexcept>
#include <utility>
#include <mutex>

// 自定义异常类（继承自 std::exception）
class BaseException : public std::exception {
private:
    std::string msg_;
    std::string file_;
    int line_;

public:
    BaseException(std::string msg, std::string  file, int line)
            : msg_(std::move(msg)), file_(std::move(file)), line_(line) {}

    [[nodiscard]] const char* what() const noexcept override {
        return msg_.c_str();
    }

    [[nodiscard]] std::string getFile() const { return file_; }
    [[nodiscard]] int getLine() const { return line_; }
};

// 封装 throw 的宏（自动注入文件名和行号）
#define THROW_EXCEPTION(msg) throw BaseException(msg, __FILE__, __LINE__)

namespace q1x {

    class error : public std::error_code {
    public:
        error(int ev, std::string msg)
            : std::error_code(ev, std::generic_category())
            ,  // 使用标准分类
            msg_(std::move(msg)) {}

        std::string message() const {
            return msg_;
        }

    private:
        std::string msg_;
    };

    // 工厂函数
    inline error make_error_code(int err_code, std::string message) {
        return error(err_code, std::move(message));
    }
}  // namespace q1x

#endif //QUANT1X_STD_EXCEPT_H
