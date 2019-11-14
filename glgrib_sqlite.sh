#!/bin/bash

set -x
set -e
\rm -f a.out
g++ $CXXFLAGS  glgrib_sqlite.cc -lsqlite3
./a.out
