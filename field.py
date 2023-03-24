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

size = 3600
windowsize = 900
laplace = np.array([
    [0, 1, 0],
    [1, -4, 1],
    [0, 1, 0]
])
win = SDL_CreateWindow(b"field",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowsize, windowsize, SDL_WINDOW_SHOWN)
surf = SDL_GetWindowSurface(win)

D = np.zeros((size, size))
H = np.zeros((size, size))
frames = 1000000
hist = []
for i in range(0, frames):
    
    #works 
    # best when < 0.7
    v =  0.7

    #wave propagation
    #spatial second derivative of D
    #D = ndimage.gaussian_filter(D, 1, mode='nearest')
    dD = ndimage.laplace(D, cval=0.0)
    #Time derivitive of H (Second Time derivative of D)
    H = H + dD
    D = D + v**2 * H
    D = D
   #geometry

    source_length = 10
    source_height = 250
    source_x = size // 3
    tracklen = size // 4
    source_y = size // 4

    #antenna
    #D[source_x:source_x + source_length, source_y: source_y + source_height] = 10 * np.cos(0.4 * i + 2 * np.pi / (450) * np.array(np.arange(0, source_height)))

    #impulse

    if(i < 1):
        D[source_x: source_x + source_length, source_y:source_y+source_height] = 100
    else:
        D[source_x: source_x + source_length, source_y:source_y+source_height] = 0

    #you can use this to measure wavelength, speed, etc
    wid = size // 20
    dep = 20
    cen = size // 3 + 13
    h = size // 2
    D[h:h+dep,0:cen - wid] = 0
    D[h:h+dep,cen + wid: size] = 0
    if(not poll()):
        break

    fast_forward = 9999
    if(i < fast_forward):
        print(f"{100 * float(i) / float(fast_forward):3.2f}%", end="\r")
        continue

    #color
    normalized = np.clip(ndimage.zoom(D, windowsize/size , order=1) / 30, -0.5, 0.5)
    blue = -255 * (np.clip(normalized, -0.5, 0) - 0.5)
    red =  255 * (np.clip(normalized, 0, 0.5) + 0.5)
    out = (red.astype(int) << 8) + blue.astype(int)

    #I/O
    pixels = ext.pixels2d(surf)
    pixels[:,:] = out.T.astype(int)
    SDL_UnlockSurface(surf)
    SDL_UpdateWindowSurface(win)


