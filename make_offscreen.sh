#!/bin/bash

gcc -DPPM=1 -DLIBPNG=1 -DFFMPEG=1 -ggdb3 -std=c99 -O0 -Wall -Wextra \
  -o offscreen.x offscreen.c -lGL -lGLU -lglut -lpng \
  -lavcodec -lswscale -lavutil 
# -I $PWD/usr/include -L $PWD/usr/lib64 -Wl,-rpath,$PWD/usr/lib64
