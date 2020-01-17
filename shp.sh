#!/bin/bash

set -x


S=h

#/usr/bin/time -p perf record \
gdb -ex="set confirm on" -ex=run -ex=quit --args \
./glgrib.x --land.on  --land.path coastlines/shp/GSHHS_${S}_L1.shp \
--land.selector "rowid <= 40000" --land.scale 1.01 \
--coast.on --coast.lines.path coastlines/shp/GSHHS_${S}_L1.shp \
--coast.lines.format shp --view.lon 0 --view.lat 0 

#--landscape.path landscape/black.bmp --landscape.on
