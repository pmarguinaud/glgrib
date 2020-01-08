#!/bin/bash

set -x

#/usr/bin/time -p perf record \
gdb -ex="set confirm on" -ex=run -ex=quit --args \
./glgrib.x --coast.on --coast.lines.format shp  --coast.lines.path coastlines/shp/GSHHS_c_L1.shp  --coast.lines.selector "rowid == 1" --grid.on --test.on


#./glgrib.x --coast.on --coast.lines.format shp  --coast.lines.path gshhs/GSHHS_shp/c/GSHHS_c_L6.shp --test.on --test.path gshhs/GSHHS_shp/c/GSHHS_c_L6.shp --test.selector "rowid == 2"


