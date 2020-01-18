#!/bin/bash

set -x

#/usr/bin/time -p perf record \
gdb -ex="set confirm on" -ex=run -ex=quit --args \
./glgrib.x --land.on  --land.layers[0].on  --land.layers[1].off  --land.layers[2].off  --land.layers[3].off  --land.layers[0].path ./coastlines/shp/GSHHS_i_L2.shp --land.layers[0].debug.on

