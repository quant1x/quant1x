#include <quant1x/test/test.h>
#include <quant1x/std/scheduler.h>
#include <quant1x/runtime/once.h>
#include <quant1x/std/time.h>
#include <quant1x/exchange/timestamp.h>
#include <quant1x/exchange/calendar.h>

static int test_number =0;

void test_once() {
    std::cout << "test_number incr" << std::endl;
    test_number+= 1;
}

TEST_CASE("cron-v1", "[runtime]") {
    runtime::global_init();
    runtime::logger_set(true, true);
    auto once = RollingOnce::create("v1",5);
    for (int i = 0; i < 10; ++i) {
        once->Do(test_once);
        spdlog::debug("test_number={}", test_number);
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
    std::this_thread::sleep_for(std::chrono::seconds(60));
    AsyncScheduler scheduler;

    // 每天8:30执行
    auto id1 = scheduler.schedule_cron("T0830", "0 44 20 * * *", [] {
        std::cout << "Daily task at 8:30" << std::endl;
    });

    // 每周一至周五每5秒钟
    auto id2 = scheduler.schedule_cron("T5s", "*/5 * * * * ?", [] {
        spdlog::debug("Every 5 seconds on weekdays");
        //std::this_thread::sleep_for(std::chrono::seconds(10));
    });

    std::this_thread::sleep_for(std::chrono::minutes(10));
    scheduler.cancel(id2);


    // 运行10秒后停止
    std::this_thread::sleep_for(std::chrono::seconds(100));
    //scheduler.stop();
    (void) id1;
    (void) id2;
}

/**
 * 在固定时间窗口内保证操作只执行一次，窗口通过 cron 表达式或时间间隔定义。
 * 每次窗口结束时自动重置，允许下个窗口重新执行。
 */
template<typename T>
class PeriodicOnce1 {
public:
    // 构造函数：接受初始化函数（支持任意可调用对象）
    template<typename Func, typename = std::enable_if_t<!std::is_same_v<std::decay_t<Func>, PeriodicOnce1>>>
    explicit PeriodicOnce1(Func&& init): init_(std::forward<Func>(init)) {}

    // 获取值，首次调用或重置后执行初始化函数
    T& get() {
        if (!done_.load(std::memory_order_acquire)) {
            std::lock_guard lock(mutex_);
            if (!done_) {
                value_.emplace(init_());  // 调用初始化函数
                spdlog::debug("value = {}", value_.value().toString());
                done_.store(true, std::memory_order_release);
            }
        }
        return *value_;
    }

    // 重置状态，允许下次调用 get 时重新初始化
    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        done_.store(false, std::memory_order_release);
        value_.reset();  // 清空存储的值
        spdlog::debug("reset....");
    }

    // 禁止拷贝和赋值
    PeriodicOnce1(const PeriodicOnce1&) = delete;
    PeriodicOnce1& operator=(const PeriodicOnce1&) = delete;

    // 允许隐式转换
    operator T&() {
        return get();
    }
    // 允许隐式转换
    operator const T&() const {
        return get();
    }
private:
    std::function<T()> init_;        // 存储初始化函数
    std::atomic<bool> done_{false};  // 初始化完成标志
    std::optional<T> value_;         // 存储泛型值
    std::mutex mutex_;               // 保护并发访问的互斥锁
};


template<typename T>
class cache1d {
public:
    template<typename Func, typename = std::enable_if_t<!std::is_same_v<std::decay_t<Func>, cache1d>>>
    explicit cache1d(Func&& init, const std::string &spec = "*/1 * * * * *") : once_(PeriodicOnce1<T>(std::forward<Func>(init))){
        task_id_  = runtime::add_task("t1", spec, [&](){once_.reset();});
        spdlog::warn("cache1d add({})", task_id_);
    }
    ~cache1d() {
        spdlog::warn("cache1d cancel({})", task_id_);
        runtime::cancel_task(task_id_);
    }

    //cache1d() = delete;

    T& get() {
        return once_.get();
    }

    // 允许隐式转换
    operator T& () {
        return once_.get();
    }
    // 允许隐式转换
    operator const T& () const {
        return once_.get();
    }
private:
    PeriodicOnce1<T> once_;
    int64_t task_id_;
};


// 获取当前日期, 这里再封装一层, 尽量不暴露内部函数
inline std::string init_current_day() {
    spdlog::debug(__FUNCTION__ );
    return api::today();
}

// 当前日期, 过0点转换
inline auto current_day = cache1d<std::string>(init_current_day);

inline exchange::timestamp init_timestamp() {
    spdlog::debug(__FUNCTION__ );
    auto now = exchange::timestamp::now();
    //return now.pre_market_time();
    spdlog::debug("now={}", now.toString());
    return now;
}


TEST_CASE("cache1d-cron", "[crontab]") {
    runtime::global_init();
    runtime::logger_set(true, true);
    auto ts_today_init = cache1d<exchange::timestamp>(init_timestamp);
    for(int i = 0; i < 5; ++i) {
        std::cout << exchange::timestamp::now().toString() << ", " << ts_today_init << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

TEST_CASE("cache1d-release", "[crontab]") {
    runtime::global_init();
    runtime::logger_set(true, true);
    auto ts_today_init2 = runtime::cache1d<exchange::timestamp>("t2", init_timestamp, "*/1 * * * * *");
    for(int i = 0; i < 5; ++i) {
        std::cout << exchange::timestamp::now().toString() << ", " << ts_today_init2 << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}