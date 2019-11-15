#!/bin/bash

CXXFLAGS="-O0 -std=c++11 -g"

set -x
set -e
\rm -f glgrib_sqlite.x
g++ $CXXFLAGS  glgrib_sqlite.cc -lsqlite3 -o glgrib_sqlite.x
./glgrib_sqlite.x
