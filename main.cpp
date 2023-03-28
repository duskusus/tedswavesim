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

    int frame = 0;
    while(poll()){
    auto frame_start = std::chrono::steady_clock::now();
    SDL_LockSurface(surf);

    int source_x = sizex / 4;
    int source_y = sizey / 2;

    D.set(source_x, source_x + sizex / 2, source_y, source_y + 10, 10 * sin(float(frame) / 10));

    D.set(0, sizex / 2, sizey/4, sizey/4 + 10, 0);
    D.set(sizex / 2 + 50, sizex, sizey/4, sizey/4 + 10, 0);
    scalar_laplacian(D, dD);
    H += dD;
    D += H * (v * v);

    n2dArray<float>D_out = D.scale(winsizex, winsizey);
    for(int i = 0; i < D_out.size; i++) {
        float Di = D_out.data[i] / 15;
        float red = -min<float>(Di, 0);
        float green = max<float>(Di, 0);
        pixels[i] = int(255 * clamp<float>(red, 0.0, 1.0)) | (int(255 * clamp<float>(green, 0.0, 1.0)) << 16);
    }
    SDL_UnlockSurface(surf);
    SDL_UpdateWindowSurface(win);
    frame ++;
    printf("Frame Took %2d ms\r", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - frame_start).count());
    }
}