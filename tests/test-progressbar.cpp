#include <atomic>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#undef max  // 避免与 std::max 冲突
#undef min  // 避免与 std::min 冲突
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

class ProgressBar {
public:
    ProgressBar(int64_t total, const std::string& prefix = "")
            : total_(total), prefix_(prefix), start_time_(std::chrono::steady_clock::now()) {
        init_color_system();
        get_terminal_size();
        start_display_thread();
    }

    ~ProgressBar() {
        stop();
        if (display_thread_.joinable()) {
            display_thread_.join();
        }
        reset_color();
    }

    void add(int64_t n = 1) {
        const int64_t new_val = current_.fetch_add(n, std::memory_order_relaxed) + n;
        update_stats(new_val);
    }

private:
    const int64_t total_;
    std::atomic<int64_t> current_{0};
    std::string prefix_;
    std::atomic<bool> running_{true};
    std::thread display_thread_;
    int term_width_ = 80;

    // 原子状态变量(缓存行对齐)
    struct alignas(64) DisplayState {
        std::atomic<int64_t> current{0};
        std::atomic<int64_t> speed{0};
        std::atomic<int> rate{0};
        std::atomic<int> last_rate{0}; // 标志最近刷新的百分比
        std::atomic<int> cost{0};
        std::atomic<int> estimate{0};
    };
    DisplayState state_;

    // 颜色控制
#ifdef _WIN32
    HANDLE hConsole_;
    WORD default_attrs_;
#else
    std::string current_color_;
#endif

    std::chrono::steady_clock::time_point start_time_;

    void init_color_system() {
#ifdef _WIN32
        hConsole_ = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole_, &csbi);
        default_attrs_ = csbi.wAttributes;
#else
        current_color_ = "";
#endif
    }

    void set_color(int64_t speed) {
        // 速度阈值配置(可根据需要调整)
        const int64_t slow = 50'000;    // <50k ops/s: 红色
        const int64_t fast = 200'000;   // >200k ops/s: 绿色

#ifdef _WIN32
        WORD color = FOREGROUND_INTENSITY;
        if (speed < slow) {
            color |= FOREGROUND_RED;
        } else if (speed < fast) {
            color |= FOREGROUND_RED | FOREGROUND_GREEN;
        } else {
            color |= FOREGROUND_GREEN;
        }
        SetConsoleTextAttribute(hConsole_, color);
#else
        if (speed < slow) {
            current_color_ = "\033[31m"; // 红色
        } else if (speed < fast) {
            current_color_ = "\033[33m"; // 黄色
        } else {
            current_color_ = "\033[32m"; // 绿色
        }
#endif
    }

    void reset_color() {
#ifdef _WIN32
        SetConsoleTextAttribute(hConsole_, default_attrs_);
#else
        current_color_ = "\033[0m";
        std::cout << current_color_;
#endif
    }

    void get_terminal_size() {
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(hConsole_, &csbi)) {
            term_width_ = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        }
#else
        struct winsize size;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == 0) {
            term_width_ = size.ws_col;
        }
#endif
        term_width_/=2;
    }

    void update_stats(int64_t current) {
        const auto now = std::chrono::steady_clock::now();
        const auto elapsed = now - start_time_;
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

        // 计算速率
        int64_t speed = 0;
        if (ms > 0) {
            speed = current * 1000 / ms;
        }

        // 更新原子状态
        state_.current.store(current, std::memory_order_relaxed);
        state_.rate.store(static_cast<int>(current * 100 / total_), std::memory_order_relaxed);
        state_.speed.store(speed, std::memory_order_relaxed);
        state_.cost.store(static_cast<int>(ms / 1000), std::memory_order_relaxed);

        if (speed > 0) {
            state_.estimate.store(
                    static_cast<int>((total_ - current) * 1000 / speed),
                    std::memory_order_relaxed
            );
        }
    }

    void start_display_thread() {
        display_thread_ = std::thread([this] {

            auto last_update = std::chrono::steady_clock::now();

            while (running_) {
                // 读取当前状态
                const int rate = state_.rate.load(std::memory_order_relaxed);
                const int last_rate = state_.last_rate.load(std::memory_order_relaxed);
                const int64_t current = state_.current.load(std::memory_order_relaxed);
                const int64_t speed = state_.speed.load(std::memory_order_relaxed);
                const int cost = state_.cost.load(std::memory_order_relaxed);
                const int estimate = state_.estimate.load(std::memory_order_relaxed);

                // 动态调整刷新率(最高60FPS)
                const auto now = std::chrono::steady_clock::now();
                if (rate < 100 && rate == last_rate && (now - last_update) < std::chrono::milliseconds(16)) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    continue;
                }

                // 构建显示内容
                std::stringstream ss;
                format_display(ss, current, rate, speed, cost, estimate);

                // 输出到控制台
                std::cout << ss.str() << std::flush;

                //last_rate = rate;
                state_.last_rate.store(rate, std::memory_order_relaxed);
                last_update = now;

                if (rate >= 100) {
                    break;
                }
            }
        });
    }

    void format_display(std::stringstream& ss,
                        int64_t current, int rate,
                        int64_t speed, int cost, int estimate) {
        get_terminal_size(); // 实时获取终端宽度
        // 计算进度条长度
        const int min_info_width = 45;
        const int bar_width = std::max(20, term_width_ - min_info_width);
        const int filled = bar_width * rate / 100;

        // 设置颜色
        set_color(speed);

        // 构建显示字符串
#ifdef _WIN32
        ss << "\r";
#else
        ss << "\r" << current_color_;
#endif

        ss << prefix_ << " "
           << std::setw(3) << rate << "% "
           << "(" << current << "/" << total_ << ") ["
           << std::string(filled, '=');

        if (filled < bar_width) {
            ss << ">" << std::string(bar_width - filled - 1, '-');
        }

        ss << "] "
           << (speed) << "/s "
           << format_timestamp_from_i64(cost) << " in: "
           << format_timestamp_from_i64(estimate);

#ifndef _WIN32
        ss << "\033[0m"; // 重置颜色
#endif
    }

    std::string format_timestamp_from_i64(int seconds) {
        int h = seconds / 3600;
        int m = (seconds % 3600) / 60;
        int s = seconds % 60;

        std::stringstream ss;
        ss << std::setfill('0')
           << std::setw(2) << h << ":"
           << std::setw(2) << m << ":"
           << std::setw(2) << s;
        return ss.str();
    }

    void stop() {
        int64_t current = state_.current.load(std::memory_order_relaxed);
        //int rate = state_.last_rate.load(std::memory_order_relaxed);
        //std::cout << std::endl;
        //std::cout << current << "/" << total_ << std::endl;
        //std::cout << "rate" << ":" << rate << std::endl;
        while(state_.last_rate.load(std::memory_order_relaxed)<100 && current == total_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        running_ = false;
    }
};

// 使用示例
int main() {
#ifdef _WIN32
    // 设置控制台输出和输入代码页为UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    //std::locale::global(std::locale(".65001"));
#endif
    constexpr int64_t total_operations = 10'000'000;
    ProgressBar bar(total_operations, "量子计算进度");

    for (int64_t i = 0; i < total_operations; ++i) {
        bar.add(1); // 每次操作都更新

        // 模拟可变工作负载
        if (i < 3'000'000) {
            std::this_thread::sleep_for(std::chrono::nanoseconds(500));
        } else if (i < 7'000'000) {
            std::this_thread::sleep_for(std::chrono::nanoseconds(200));
        } else {
            std::this_thread::sleep_for(std::chrono::nanoseconds(50));
        }
    }
    //std::this_thread::sleep_for(std::chrono::seconds(50));
    return 0;
}