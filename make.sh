#!/bin/bash


gcc -g -c load.c -leccodes #-I$HOME/install/eccodes-2.12.0/include 
g++  -std=c++11  -g -o glgrib.x glgrib.cc  glgrib_png.cc glgrib_fb.cc  glgrib_render.cc  glgrib_x11.cc  shader.cc load.o -lGLEW -lGL -lEGL -lglfw -leccodes -lpng #-L$HOME/install/eccodes-2.12.0/lib -Wl,-rpath,$HOME/install/eccodes-2.12.0/lib -I ./usr/include/ -L ./usr/lib64  -Wl,-rpath,$PWD/usr/lib64

