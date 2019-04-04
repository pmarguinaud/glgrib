#!/bin/bash

# -lavcodec -lswscale -lavutil \
gcc -DPPM=0 -DLIBPNG=1 -DFFMPEG=0 -ggdb3 -std=c99 -O0 -Wall -Wextra \
  -o offscreen.x offscreen.c -lGL -lGLU -lglut -lpng \
  -I $PWD/usr/include -L $PWD/usr/lib64 -Wl,-rpath,$PWD/usr/lib64
