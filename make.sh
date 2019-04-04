#!/bin/bash

gcc -g -c load.c -L$HOME/install/eccodes-2.12.0/lib -leccodes -I$HOME/install/eccodes-2.12.0/include 
g++ -g -o glgrib.x *.cc load.o -lGLEW -lGL -lglfw -leccodes -L$HOME/install/eccodes-2.12.0/lib -Wl,-rpath,$HOME/install/eccodes-2.12.0/lib -I ./usr/include/ -L ./usr/lib64  -Wl,-rpath,$PWD/usr/lib64

