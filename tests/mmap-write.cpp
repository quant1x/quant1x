#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "test-mmap.h"
#include <q1x/runtime/core.h>

int main(int argc, const char * const argv[]) {
    (void)argc;
    (void)argv;
    runtime::global_init();
    // 创建控制台和文件 sink
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/multi.log");

    // 组合多个 sink 到日志器
    spdlog::logger logger("multi_sink", {console_sink, file_sink});
    spdlog::info("This message goes to both console and file");
    const char * const mmap_filename = "d:/runtime/temp/data.mmap";
    const int count = 10;
    //auto dataSize = sizeof(Market) * count;
    MemObject<Market> cache(mmap_filename);
    std::cout << *cache.get_header() << std::endl;
    auto * ms = cache.toSlice();
    for(int i = 0; i < count; i++) {
        std::cout << ms[i] << std::endl;
        ms[i].a = i+100000;
        ms[i].b = i*100 +i;
    }
    auto newIndex = int(cache.Add(1));
    ms[newIndex].a = newIndex;
    ms[newIndex].b = newIndex;
    std::cout << newIndex << ": " << ms[newIndex] << std::endl;
    std::cout << *cache.get_header() << std::endl;
    //cache.add(count);
}