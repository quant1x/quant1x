#ifndef QUANT1X_LABS_H
#define QUANT1X_LABS_H

#include <immintrin.h> // for sse and avx
#include <stdint.h>    // for int
#include <stdbool.h>   // for bool

void b8x32_and_none(const bool *a, const bool *b, bool *c, int n)
{
    int simd_bandwidth = 256;
    int bits = sizeof(*a)*8;
    int lanes = simd_bandwidth/bits;
    bits = 8;
    lanes = 32;
    int epoch = n / lanes;
    int remain = n % lanes;

    for (int i = 0; i < epoch; i += 32)
    {
        c[i+0] = a[i+0] && b[i+0];
        c[i+1] = a[i+1] && b[i+1];
        c[i+2] = a[i+2] && b[i+2];
        c[i+3] = a[i+3] && b[i+3];
        c[i+4] = a[i+4] && b[i+4];
        c[i+5] = a[i+5] && b[i+5];
        c[i+6] = a[i+6] && b[i+6];
        c[i+7] = a[i+7] && b[i+7];
        c[i+8] = a[i+8] && b[i+8];
        c[i+9] = a[i+9] && b[i+9];
        c[i+10] = a[i+10] && b[i+10];
        c[i+11] = a[i+11] && b[i+11];
        c[i+12] = a[i+12] && b[i+12];
        c[i+13] = a[i+13] && b[i+13];
        c[i+14] = a[i+14] && b[i+14];
        c[i+15] = a[i+15] && b[i+15];
        c[i+16] = a[i+16] && b[i+16];
        c[i+17] = a[i+17] && b[i+17];
        c[i+18] = a[i+18] && b[i+18];
        c[i+19] = a[i+19] && b[i+19];
        c[i+20] = a[i+20] && b[i+20];
        c[i+21] = a[i+21] && b[i+21];
        c[i+22] = a[i+22] && b[i+22];
        c[i+23] = a[i+23] && b[i+23];
        c[i+24] = a[i+24] && b[i+24];
        c[i+25] = a[i+25] && b[i+25];
        c[i+26] = a[i+26] && b[i+26];
        c[i+27] = a[i+27] && b[i+27];
        c[i+28] = a[i+28] && b[i+28];
        c[i+29] = a[i+29] && b[i+29];
        c[i+30] = a[i+30] && b[i+30];
        c[i+31] = a[i+31] && b[i+31];
        a+= lanes;
        b+= lanes;
        c+= lanes;
    }
    for(int k = 0; k < remain; k++)
    {
        *c = *a && *b;
        a +=1;
        b +=1;
        c +=1;
    }
}

void f32x8_add_none(const float *a, const float *b, float *c, int n)
{
    int simd_bandwidth = 256;
    int bits = sizeof(*a)*8;
    int lanes = simd_bandwidth/bits;
    bits = 32;
    lanes = 8;
    int epoch = n / lanes;
    int remain = n % lanes;

    for (int i = 0; i < epoch; i += 8)
    {
        c[i+0] = a[i+0] + b[i+0];
        c[i+1] = a[i+1] + b[i+1];
        c[i+2] = a[i+2] + b[i+2];
        c[i+3] = a[i+3] + b[i+3];
        c[i+4] = a[i+4] + b[i+4];
        c[i+5] = a[i+5] + b[i+5];
        c[i+6] = a[i+6] + b[i+6];
        c[i+7] = a[i+7] + b[i+7];
        a+= lanes;
        b+= lanes;
        c+= lanes;
    }
    for(int k = 0; k < remain; k++)
    {
        *c = *a + *b;
        a +=1;
        b +=1;
        c +=1;
    }
}

#endif //QUANT1X_LABS_H
