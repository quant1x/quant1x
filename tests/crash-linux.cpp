#define BACKWARD_HAS_DW 1
#include "backward.hpp"

#include <iostream>

namespace {
    backward::SignalHandling sh; // 自动注册信号处理器
}


void bar() {
    int* p = nullptr;
    *p = 42; // 真正访问空指针
}

void crash_function() {
    bar();
}

int main() {
    std::cout << "程序启动成功" << std::endl;
    crash_function();
    return 0;
}
