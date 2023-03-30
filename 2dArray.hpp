#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <cstring>
#include <cassert>
#include <immintrin.h>
#include "util.h"
#include <future>

#define ALIGN_MASK uint64_t(0xffffffffffffffe0)
void *align(void *in) {
    void *out = (void *) ((((uint64_t)in) & ALIGN_MASK) + 32);
    assert(out != 0);
    return out;
}
template <typename T>
class n2dArray{
    public:
    const uint32_t M; //rows
    const uint32_t N; //columns
    public:
    const uint32_t size = M * N;
    T *data = nullptr;
    T *allocation = nullptr;

    n2dArray(uint32_t p_M, uint32_t p_N, T *p_data = nullptr) :
    M(p_M), N(p_N) {
        allocation = (T*)malloc(sizeof(T) * size + 64);
        data = (T *)align(allocation);
        printf("data: %lu\n", data);
        if(p_data) {
            memcpy(data, p_data, sizeof(T) * M * N);
        }
    }
    n2dArray(const n2dArray &b):
    M(b.M), N(b.N) {
        allocation = (T*)malloc(sizeof(T) * size + 64);
        data = (T *) align(allocation);
        memcpy(allocation, b.allocation, b.size * sizeof(T) + 64);
    }
    ~n2dArray() {
        free(allocation);
    }
    void operator=(const n2dArray<T> &b) {
        assert(b.M == M && b.N == N);
        memcpy(allocation, b.allocation, size * sizeof(T) + 64);
    }
    __attribute__((always_inline)) T &operator[](int i) {
        return data[i];
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

    static void thread_scalar_laplacian(const n2dArray<T> &in, n2dArray<T> &out, int istart, int iend) {
        const T *d = in.data;

        for(int i = istart; i < iend; i++) {
            out.data[i] = -4 * d[i] + d[i - 1] + d[i + 1] + d[i - in.N] + d[i + in.N];
        }
        return;
    }
    friend void scalar_laplacian(const n2dArray<T> &in, n2dArray<T> &out, int threads, std::future<void> *futures) {
        
        assert(in.M == out.M & in.N == out.N);
        //thread_scalar_laplacian(in, out, in.N, in.size - in.N);
        auto op = [&in, &out](int istart, int iend){thread_scalar_laplacian(in, out, istart, iend);};

        int safe_size = in.size - 2 * in.N;

        for(int t = 0; t < threads; t++) {
            futures[t] = std::async(std::launch::async, op, t * safe_size/threads + in.N, (t + 1) * safe_size/threads + in.N);
        }
        futures[threads-1].wait();
    }
    void operator+=(const n2dArray<T> &b) {
        assert(M == b.M && N == b.N);
        for(int i = 0; i < size; i++) {
            data[i] += b.data[i];
        }
    }
    T at(int x, int y) {
        return data[(x % N) * N + (y % M)];
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