// 调度原语开销探针: sleep_for / yield / Sleep(1) 的真实耗时
//
// 存在理由: 这是定位"Windows x86_64 上 C++ 比 Rust 慢数倍"的关键工具.
// 名义 50us 的休眠在各实现下差异极大:
//   * C++ std::this_thread::sleep_for(50us) 在 MSVC 上向上取整为 Sleep(1),
//     受系统默认 15.6ms 定时器粒度支配 → 实测约 15.6ms(名义值的 311 倍)
//   * Rust thread::sleep(50us)                                  → 实测约 0.55ms
//   * CreateWaitableTimerExW(CREATE_WAITABLE_TIMER_HIGH_RESOLUTION) → 实测约 0.53ms
// 相差近 30 倍, 直接决定了无锁队列第三级退避的停摆时长.
//
// 本探针在 C++ 侧同时给出这三种手段的实测值, 便于在换机器 / 换编译器 / 换
// 系统电源策略后复核结论, 也用于验证 safe::sleep_for_microseconds 是否仍生效
// (若其耗时回落到 15ms 量级, 说明高精度定时器不可用, 队列退避会回归慢路径).
//
// 用法: 见 scripts/msvc_sched_cost_probe.bat

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <windows.h>

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <thread>

#include <quant1x/base/safe.h>

namespace {

using clk = std::chrono::steady_clock;

void bench(const char* name, int n, void (*fn)()) {
    fn();  // 预热: 首次调用可能触发惰性初始化(定时器/CRT), 计入会污染均值
    const auto a = clk::now();
    for (int i = 0; i < n; ++i) {
        fn();
    }
    const auto b = clk::now();
    const double us = std::chrono::duration<double, std::micro>(b - a).count() / n;
    std::printf("%-40s x%6d : avg %10.1f us per call\n", name, n, us);
}

void f_sleep_for_50us() { std::this_thread::sleep_for(std::chrono::microseconds(50)); }
void f_sleep0() { ::Sleep(0); }
void f_switch() { ::SwitchToThread(); }
void f_yield() { std::this_thread::yield(); }

// safe::sleep_for_microseconds(50) —— 队列退避实际使用的路径
void f_safe_sleep_50us() { safe::sleep_for_microseconds(50); }

// 对照: 未加 CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 的普通定时器, 验证该 flag
// 才是精度差异的来源(而非"可等待定时器"这一机制本身).
void f_plain_timer_50us() {
    HANDLE t = ::CreateWaitableTimerW(nullptr, FALSE, nullptr);
    if (t) {
        LARGE_INTEGER due;
        due.QuadPart = -500;  // 50us, 单位 100ns, 负值表示相对时间
        ::SetWaitableTimer(t, &due, 0, nullptr, nullptr, FALSE);
        ::WaitForSingleObject(t, INFINITE);
        ::CloseHandle(t);
    }
}

}  // namespace

int main() {
    std::printf("=== 调度原语实测开销 (名义值仅供参考, 实际受系统定时器粒度支配) ===\n\n");
    bench("sleep_for(50us) [应规避]", 200, f_sleep_for_50us);
    bench("safe::sleep_for_microseconds(50) [当前]", 2000, f_safe_sleep_50us);
    bench("plain waitable timer (50us) [对照]", 2000, f_plain_timer_50us);
    bench("Sleep(0)", 20000, f_sleep0);
    bench("SwitchToThread()", 20000, f_switch);
    bench("std::this_thread::yield()", 20000, f_yield);

    std::printf("\n判定: safe::sleep_for_microseconds 应显著快于 sleep_for(50us).\n");
    std::printf("若两者接近(均在 15ms 量级), 说明高精度定时器在当前系统不可用,\n");
    std::printf("队列第三级退避会回归 15.6ms 粒度, 多线程吞吐将大幅下降.\n");
    return 0;
}
