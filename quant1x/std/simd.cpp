#include <quant1x/std/simd.h>

using vector_type = std::vector<double, xsimd::default_allocator<double>>;
void sample_mean(const vector_type& a, const vector_type& b, vector_type& res)
{
    std::size_t size = a.size();
    constexpr std::size_t simd_size = xsimd::simd_type<double>::size;
    std::size_t vec_size = size - size % simd_size;

    for(std::size_t i = 0; i < vec_size; i += simd_size)
    {
        auto ba = xsimd::load_aligned(&a[i]);
        auto bb = xsimd::load_aligned(&b[i]);
        auto bres = (ba + bb) / 2.;
        bres.store_aligned(&res[i]);
    }
    for(std::size_t i = vec_size; i < size; ++i)
    {
        res[i] = (a[i] + b[i]) / 2.;
    }
}