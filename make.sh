#!/bin/bash

gcc -g -c glgrib.c 
g++ -g -o tutorial04bis.x *.cpp glgrib.o -lGLEW -lGL -lglfw -leccodes

