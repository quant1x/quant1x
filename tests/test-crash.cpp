#include <quant1x/runtime/crash.h>
#include <quant1x/runtime/core.h>

void bar() {
    int* p = nullptr;
    *p = 42; // 真正访问空指针
}

void foo() {
    bar();
}

int main() {
    runtime::global_init();
    runtime::logger_set(true, true);
    crash::InitCrashHandler();

    //spdlog::info("程序启动成功，即将触发崩溃...");

    foo(); // 触发崩溃

    return 0;
}
