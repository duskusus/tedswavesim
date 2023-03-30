#include <SDL2/SDL.h>

#include <chrono>
#include <iostream>
#include <vector>

#include "2dArray.hpp"

bool poll() {
    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        if(e.type == SDL_QUIT) {
            return false;
        }
    }
    return true;
}

int main()
{
    SDL_Init(SDL_INIT_VIDEO);
    int sizex = 1800;
    int sizey = 1800;

    int winsizex = 900;
    int winsizey = 900;

    SDL_Window *win = SDL_CreateWindow("ted's wave sim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, winsizex, winsizey, SDL_WINDOW_SHOWN);
    SDL_Surface *surf = SDL_GetWindowSurface(win);
    uint32_t * pixels = (uint32_t *)surf->pixels;
    
    const float v = 0.5;

    n2dArray<float> D(sizex, sizey);
    n2dArray<float> H(sizex, sizey);
    n2dArray<float> dD(sizex, sizey);

    n2dArray<float> nD(sizex, sizey);
    n2dArray<float> nH(sizex, sizey);
    n2dArray<float> ndD(sizex, sizey);

    n2dArray<float>D_out(winsizex, winsizey);

    std::future<void> futures[16];
    int threads = 6;

    int frame = 0;
    auto frame_start = std::chrono::steady_clock::now();
    while(poll()){
    
    const int source_x = sizex / 4;
    const int source_y = sizey / 2;

    D.set(source_x, source_x + sizex / 2, source_y, source_y + 10, 10 * sin(float(frame) /20));
    D.set(0, sizex / 2, sizey/4, sizey/4 + 10, 0);
    D.set(sizex / 2 + 50, sizex, sizey/4, sizey/4 + 10, 0);

    //scalar_laplacian(D, dD, threads, futures);
    //H += dD;
    //D += H * (v * v);
    __m256 min4 = _mm256_set1_ps(-4);
    __m256 v2 = _mm256_set1_ps(v * v);

    __m256 di = _mm256_load_ps(&D[D.N-8]);
    __m256 ndi;

    __m256 l_up;
    __m256 l_left;
    __m256 l_right = _mm256_load_ps(&D[D.N]);
    __m256 l_down;

    for(int i = D.N; i < D.size - 2 * D.N; i+=8) {
        //nD[i] = D[i];
        l_left = di;
        di = l_right;
        l_right = _mm256_load_ps(&D[i + 8]);
        __m256 h = _mm256_load_ps(&H[i]);
        //float laplace = -4 * D[i] + D[i - D.N] + D[i - 1] + D[i+1] + D[i + D.N];
        //H[i] += laplace;
        l_up = _mm256_load_ps(&D[i-D.N]);
        l_down = _mm256_load_ps(&D[i+D.N]);

        h = _mm256_fmadd_ps(di, min4, h);
        h = _mm256_add_ps(h, l_up);
        h = _mm256_add_ps(h, l_left);
        h = _mm256_add_ps(h, l_right);
        h = _mm256_add_ps(h, l_down);
        _mm256_store_ps(&H[i], h);

        //nD[i] += H[i] * v2;
        __m256 ndi = _mm256_fmadd_ps(h, v2, di);
        _mm256_store_ps(&nD[i], ndi);

    }
    D = nD;

    SDL_LockSurface(surf);
    for(int y = 0; y < winsizey; y++) {
        for(int x = 0; x < winsizex; x++) {
        const int smalli = x + y * winsizex;
        const int round_x = (x * sizex + sizex / 2) / winsizex;
        const int round_y = (y * sizey + sizey / 2) / winsizey;
        const int i = round_x + round_y * sizex;
        float Di = D[i] / 15;
        float red = -min<float>(Di, 0);
        float green = max<float>(Di, 0);
        pixels[smalli] = int(255 * clamp<float>(red, 0.0, 1.0)) | (int(255 * clamp<float>(green, 0.0, 1.0)) << 8);
        }
    }
    SDL_UnlockSurface(surf);
    SDL_UpdateWindowSurface(win);
    frame ++;
    }
    float avg = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - frame_start).count();
    avg = avg / float(frame);
    printf("Frame Took %f ms\n", avg);
}

//old scaling takes 8.3 ms / frame at 1800px
//new scaling takes 7.9
//before avx 7.5
