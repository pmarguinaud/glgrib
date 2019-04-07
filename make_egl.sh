#!/bin/bash

c++ -DUSE_GLE -g -o egl.x  egl.cc -lGL -lEGL -lpng
