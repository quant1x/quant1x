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
#include <filesystem>

struct pos {
    int x, y;
    friend std::ostream& operator<<(std::ostream& os, const pos& obj) {
        os << "x: " << obj.x << " y: " << obj.y;
        return os;
    }
};

int main() {
    namespace data = cista::offset;
    namespace fs = std::filesystem;
    constexpr auto const MODE =  // opt. versioning + check sum
            cista::mode::WITH_VERSION | cista::mode::WITH_INTEGRITY;


    using pos_map = data::hash_map<int, pos>;
    size_t fileSize = 0;
    if (fs::exists(mmap_filename)) {
       fileSize= std::filesystem::file_size(mmap_filename);
    }
    if (fileSize == 0) {
        auto positions = pos_map{{1, pos{10,20}}, {2, pos{2,3}}};
        size_t length = positions.size();
        std::cout << "length = " << length << std::endl;
        cista::buf mmap{cista::mmap{mmap_filename}};
        cista::serialize<MODE>(mmap, positions);
    } else{
        auto b = cista::mmap(mmap_filename, cista::mmap::protection::MODIFY);
        auto positions = cista::deserialize<pos_map, MODE>(b);
        auto v = positions->at(1);
        v.y+=1;
        std::cout << v << std::endl;
        //cista::buf mmap{b};
        //cista::serialize<MODE>(mmap, positions);
        std::this_thread::sleep_for(std::chrono::seconds(1));

    }

    return 0;
}


//int main_v0() {
//    namespace data = cista::offset;
//    constexpr auto const MODE =  // opt. versioning + check sum
//            cista::mode::WITH_VERSION | cista::mode::WITH_INTEGRITY;
//
//    struct pos { int x, y; };
//    using pos_map =  // Automatic deduction of hash & equality
//            data::hash_map<data::vector<pos>,
//                    data::hash_set<data::string>>;
//
//    {  // Serialize.
//        auto positions =
//                pos_map{{{{1, 2}, {3, 4}}, {"hello", "cista"}},
//                        {{{5, 6}, {7, 8}}, {"hello", "world"}}};
//        cista::buf mmap{cista::mmap{"data"}};
//        cista::serialize<MODE>(mmap, positions);
//    }
//
//// Deserialize.
//    auto b = cista::mmap("data", cista::mmap::protection::READ);
//    auto positions = cista::deserialize<pos_map, MODE>(b);
//
//    return 0;
//}