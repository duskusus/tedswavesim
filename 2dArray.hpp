#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <cstring>
#include <cassert>
#include <immintrin.h>
#include "util.h"

inline float dot8(__m256 v1, __m256 v2)
{
    __m256 v3 = _mm256_mul_ps(v1, v2);
    __m128 l = _mm256_extractf128_ps(v3, 0);
    __m128 h = _mm256_extractf128_ps(v3, 1);
    __m128 res = _mm_hadd_ps(l, h);
    return res[0] + res[1] + res[2] + res[3];
}
template <typename T>
class n2dArray{
    const uint32_t M; //rows
    const uint32_t N; //columns
    public:
    const uint32_t size = M * N;
    T *data = nullptr;
    n2dArray(uint32_t p_M, uint32_t p_N, T *p_data = nullptr) :
    M(p_M), N(p_N) {
        data = new T[M * N];
        
        if(p_data) {
            memcpy(data, p_data, sizeof(T) * M * N);
        }
    }
    n2dArray(const n2dArray &b):
    M(b.M), N(b.N) {
        data = new T[M * N];
        memcpy(data, b.data, b.size * sizeof(T));
    }
    ~n2dArray() {
        delete[] data;
    }
    void operator=(const n2dArray<T> &b) {
        assert(b.M == M && b.N == N);
        memcpy(data, b.data, size * sizeof(T));
    }
    n2dArray operator*(T x) {
        n2dArray out = *this;
        for(int i = 0; i < size; i++) {
            out.data[i] *= x;
        }
        return out;
    }

    void set(uint32_t ymin, uint32_t ymax, uint32_t xmin, uint32_t xmax, T value) {
        for(int x = xmin; x < xmax; x++) {
            for(int y = ymin; y < ymax; y++) {
                data[x * N + y] = value;
            }
        }
    }

    friend n2dArray<T> convolve(const n2dArray<T> &r1, const n2dArray<T> &r2) {

        assert(r2.M % 2 == 1 & r2.N % 2 == 1);

        n2dArray<T> result(r1.M, r1.N);

        //rows of r1
        for(uint32_t i = 0; i < r1.M - 1; i++) {
            //columns of r1i + x
            for(uint32_t j = 0; j < r1.N - 1; j++) {

                T sum = 0;

                //rows of r2
                for(int x = 0; x < r2.M; x++) {
                    //columns of r2
                    for(int y = 0; y < r2.N; y++) {
                        const uint32_t index1 = min(y+j, r1.M) * r1.N + min(x + i, r1.N);
                        const uint32_t index2 = x * r2.N + y;
                        sum += r1.data[index1] * r2.data[index2];
                    }
                }
                uint32_t index = i * r1.N + j;
                result.data[index] = sum;

            }
        }
        return result;
    }

    friend void scalar_laplacian(const n2dArray<T> &in, n2dArray<T> &out) {
        
        assert(in.M == out.M & in.N == out.N);

        const __m256 kern = _mm256_set_ps(0, 1, 0, 1, -4, 1, 0, 1);
        const T *d = in.data;
        for(int i = 1; i < in.M - 1; i++) {
            for(int j = 1; j < in.N - 1; j++) {
                const uint32_t index = i * in.N + j;
                //const __m256 vec = _mm256_set_ps(0, d[index - in.N], 0, d[index - 1], d[index], d[index + 1], 0, d[index + in.N]);
                //out.data[index] = dot8(vec, kern);

                out.data[index] = -4 * d[index] + d[index - 1] + d[index + 1] + d[index - in.N] + d[index + in.N];
                }
        }
    }
    void operator+=(const n2dArray<T> &b) {
        assert(M == b.M && N == b.N);
        for(int i = 0; i < size; i++) {
            data[i] += b.data[i];
        }
    }
    T at(int x, int y) {
        assert(x < N && y < M);
        return data[x * N + y];
    }
    friend T sum(const n2dArray<T> &in) {
        T r_sum = 0;
        for(int i = 0; i < in.size; i++) {
            r_sum += in.data[i];
        }
        return r_sum;
    }
    friend n2dArray<T> operator+(const n2dArray<T> &a, const n2dArray<T> &b) {
        assert(a.M == b.M && a.N == b.N);
        n2dArray<T> out(a.M, a.N);
        for(uint32_t i = 0; i < out.size; i++) {
            out.data[i] = a.data[i] + b.data[i];
        }
        return out;
    }
    n2dArray<T> scale(uint32_t new_wid, uint32_t new_height) {
        n2dArray<T> out(new_wid, new_height);
        T *d = out.data;
        for(int x = 0; x < out.M; x++) {
            for(int y = 0; y < out.N; y++) {
                d[x * out.N + y] = at(N * float(x) / out.N, M * float(y) / out.M);
            }
        }
        return out;
    }
};