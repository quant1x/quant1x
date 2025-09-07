#include <quant1x/runtime/core.h>
#include <thread>
#include "test-mmap.h"

int main(int argc, const char * const argv[]) {
    (void)argc;
    (void)argv;
    runtime::global_init();
    const char * const mmap_filename = "d:/runtime/temp/data.mmap";
    //const int count = 10;
    //auto dataSize = sizeof(Market) * count;
    MemObject<Market> cache(mmap_filename);
    Market * ms = cache.toSlice();
    CacheHeader *header = cache.get_header();
    int begin = 0;
    auto cap = header->arrayCap;
    for(int t = 0; t < 1000; t++) {
        if (header->arrayCap != cap) {
            cache.remap();
            header = cache.get_header();
            ms = cache.toSlice();
            begin = cap;
            cap = header->arrayCap;
            std::cout << *cache.get_header() << std::endl;
        }
        for (uint32_t i = begin; i < cap; i++) {
            std::cout << ms[i] << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    //cache.add(count);
}