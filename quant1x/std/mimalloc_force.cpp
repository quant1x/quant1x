// This translation unit forces inclusion of mimalloc's new/delete overrides
// when the project enables mimalloc in the "new_delete" mode. Including
// the header in a single TU ensures the overridden operators are available
// to the linker on all platforms (especially Windows when using static libs).

#if defined(MI_OVERRIDE_NEW_DELETE) || defined(MI_OVERRIDE)
// On Windows and certain configurations mimalloc provides special headers to
// override global new/delete. Prefer mimalloc-new-delete.h which properly
// emits the operator overrides.
#include <mimalloc-new-delete.h>
#endif

// Empty symbol to avoid this translation unit being optimized away.
// Use a linkage-spec block instead of a single 'extern "C"' declaration with
// an initializer to avoid gcc/clang warning about 'initialized and declared extern'.
#ifdef __cplusplus
extern "C" {
#endif

int quant1x_mimalloc_force_symbol = 0;

#ifdef __cplusplus
}
#endif
