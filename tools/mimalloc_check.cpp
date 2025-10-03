#include <cstdio>
#include <cstdlib>
#include <cstring>

#if defined(_WIN32)
#include <windows.h>
#endif

#include <mimalloc.h>

int main() {
    std::printf("mimalloc_check: starting\n");
    // Print initial stats
    std::printf("--- mimalloc stats (initial) ---\n");
    mi_stats_print(NULL);

    // Allocate with C malloc
    std::printf("\nAllocating with malloc(64)\n");
    void* p = std::malloc(64);
    if (!p) {
        std::printf("malloc failed\n");
        return 1;
    }
    std::memset(p, 0xA5, 64);

    // Allocate with operator new[] to test C++ new/delete routing
    std::printf("\nAllocating with new int[10000]\n");
    int *arr = nullptr;
    try {
        arr = new int[10000];
        for (int i = 0; i < 10000; ++i) arr[i] = i;
    } catch (...) {
        std::printf("new[] failed\n");
    }

    // Allocate with mimalloc API directly
    std::printf("\nAllocating with mi_malloc(128)\n");
    void* q = mi_malloc(128);
    if (!q) {
        std::printf("mi_malloc failed\n");
        // continue, we still want to print stats
    }

    // Print stats after allocations
    std::printf("\n--- mimalloc stats (after allocations) ---\n");
    mi_stats_print(NULL);

    // Free allocations
    if (q) mi_free(q);
    if (arr) delete [] arr;
    std::free(p);

    std::printf("\n--- mimalloc stats (after frees) ---\n");
    mi_stats_print(NULL);

    std::printf("mimalloc_check: done\n");
    return 0;
}
