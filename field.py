#!/usr/bin/python3

import numpy as np
import scipy.signal as signal
import scipy.ndimage as ndimage
import time
import sys
from sdl2 import *
import sdl2.ext as ext
import ctypes

def poll():
    e = SDL_Event()
    while(SDL_PollEvent(ctypes.byref(e)) != 0):
        if(e.type == SDL_QUIT):
            return False
    return True
SDL_Init(SDL_INIT_VIDEO)

size = 900
windowsize = 900
laplace_kernel = np.array([
    [0, 1, 0],
    [1, -4, 1],
    [0, 1, 0]
])

win = SDL_CreateWindow(b"field",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowsize, windowsize, SDL_WINDOW_SHOWN)
surf = SDL_GetWindowSurface(win)
pixels = ext.pixels2d(surf)

D = np.zeros((size, size))
H = np.zeros((size, size))
frames = 1000000
hist = []
for i in range(0, frames):
    
    #works best when < 0.7
    v =  0.5

    #wave propagation
    #spatial second derivative of D
    #uncomment the following line to allow higher values for v
    #D = ndimage.gaussian_filter(D, 1, mode='nearest')
    start = time.perf_counter()   
    dD = ndimage.convolve(D, laplace_kernel, mode='nearest')
    
    #Time derivitive of H (Second Time derivative of D)
    H = H + dD
    D = D + v**2 * H

   #geometry
    source_width = 10
    source_height = 10
    source_x = size // 3
    tracklen = size // 4
    source_y = size // 4

    #antenna
    D[source_x:source_x + source_width, source_y: source_y + source_height] = 10 * np.cos(0.4 * i + 2 * np.pi / (450) * np.array(np.arange(0, source_height)))

    #impulse

    #if(i < 1):
    #    D[source_x: source_x + source_width, source_y:source_y+source_height] = 100
    #else:
    #    D[source_x: source_x + source_width, source_y:source_y+source_height] = 0

    #wall with slit
    wid = size // 20
    dep = 20
    cen = size // 3 + 13
    h = size // 2
    D[h:h+dep,0:cen - wid] = 0
    D[h:h+dep,cen + wid: size] = 0

    #input, this needs to be here so we don't skip it when fast forwarding
    if(not poll()):
        break

    fast_forward = 0
    if(i < fast_forward):
        print(f"fast forward: {100 * float(i) / float(fast_forward):3.2f}%", end="\r")
        continue

    #color
    dmax = np.max(D)
    dmin = np.min(D)
    drange = dmax - dmin
    normalized = np.clip(ndimage.zoom(D, windowsize/size , order=1) / drange, -0.5, 0.5)
    blue = -255 * (np.clip(normalized, -0.5, 0) - 0.5)
    red =  255 * (np.clip(normalized, 0, 0.5) + 0.5)
    out = (red.astype(int) << 8) + blue.astype(int)

    #I/O
    pixels[:,:] = out.T.astype(int)
    print(f"Frame took {(time.perf_counter() - start) * 1000 : 3.3f} ms", end="\r")
    SDL_UnlockSurface(surf)
    SDL_UpdateWindowSurface(win)
    print((time.perf_counter() - start) * 1000)
    


