//#define BACKWARD_HAS_DWARF 0
//#define BACKWARD_SYSTEM_UNKNOWN 0
//#define BACKWARD_SYSTEM_LINUX 0
//#define BACKWARD_SYSTEM_WINDOWS 1  // 强制启用Windows模式
//#define BACKWARD_SYSTEM_DARWIN 0
//#define BACKWARD_HAS_UNWIND 1
#include "backward.hpp"
#include <iostream>

namespace {
    backward::SignalHandling sh;
}

void crash_function() {
    volatile int* a = nullptr;
    *a = 1;  // 故意制造崩溃
}

int main() {
    std::cout << "程序开始运行..." << std::endl;
    crash_function();
    int a = 0;
    a++;
    ++a;
    std::cout<< a << std::endl;
    return 0;
}