#include <cstdio>
#include <cstdlib>
#include <cstring>

#if defined(_WIN32)
#include <windows.h>
#endif

#include <mimalloc.h>

int main() {
    std::printf("mimalloc_check: starting\n");

    // Allocate some memory via standard malloc/new to exercise redirection
    void* p = std::malloc(64);
    if (!p) {
        std::printf("malloc failed\n");
        return 1;
    }
    std::memset(p, 0xA5, 64);
    std::free(p);

    // Also use mimalloc API directly
    void* q = mi_malloc(128);
    if (!q) {
        std::printf("mi_malloc failed\n");
        return 2;
    }
    mi_free(q);

    // Print mimalloc stats to stdout; this will show whether mimalloc is active
    std::printf("Calling mi_stats_print()...\n");
    mi_stats_print(NULL);

    std::printf("mimalloc_check: done\n");
    return 0;
}
