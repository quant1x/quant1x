// 手动处理字节序检测
#if defined(_WIN32) || defined(_WIN64)
#define CISTA_LITTLE_ENDIAN 1
#elif defined(__LINUX__)
#include <endian.h>
#define IS_LITTLE_ENDIAN (__BYTE_ORDER == __LITTLE_ENDIAN)
#elif defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#endif
#include <cista/serialization.h>
#include <cista/mmap.h>
#include <quant1x/runtime/core.h>
#include <cista/containers.h>
#include <cista/serialization.h>
#include <cista/mmap.h>
#include <cista/strong.h>
#include <iostream>

const char * const mmap_filename = "d:/runtime/temp/data.mmap";
#include <cista/containers.h>
#include <cista/serialization.h>
#include <cista/mmap.h>
#include <cista/strong.h>
#include <iostream>

struct pos {
    int x, y;
    friend std::ostream& operator<<(std::ostream& os, const pos& obj) {
        os << "x: " << obj.x << " y: " << obj.y;
        return os;
    }
};

int main() {
    namespace data = cista::offset;
    constexpr auto const MODE =  // opt. versioning + check sum
            cista::mode::WITH_VERSION | cista::mode::WITH_INTEGRITY;


    using pos_map = data::hash_map<int, pos>;

//    {  // Serialize.
//        auto positions =
//                pos_map{{1, pos{1,2}}, {2, pos{2,3}}};
//        cista::buf mmap{cista::mmap{mmap_filename}};
//        cista::serialize<MODE>(mmap, positions);
//    }

    {// Deserialize.
        auto b = cista::mmap(mmap_filename, cista::mmap::protection::READ);
        auto positions = cista::deserialize<pos_map, MODE>(b);
        for (int i = 0; i < 1000; i++) {
            auto v = positions->at(1);
            std::cout << v << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    return 0;
}
