#ifndef QUANT1X_TEST_MMAP_H
#define QUANT1X_TEST_MMAP_H

#include <q1x/std/object.h>

#pragma pack(push, 1)  // 确保1字节对齐
struct Market {
    int a;
    int b;
    friend std::ostream &operator<<(std::ostream &os, const Market &market) {
        os << "a: " << market.a << " b: " << market.b;
        return os;
    }
};
#pragma pack(pop)  // 恢复默认对齐方式

#endif //QUANT1X_TEST_MMAP_H
