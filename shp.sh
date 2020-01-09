#!/bin/bash

set -x

#/usr/bin/time -p perf record \
#gdb -ex="set confirm on" -ex=run -ex=quit --args \
#./glgrib.x --coast.on --coast.lines.format shp  --coast.lines.path coastlines/shp/GSHHS_c_L1.shp  --coast.lines.selector "rowid == 1" --grid.on --test.on
#./glgrib.x --coast.on --coast.lines.format shp  --coast.lines.path gshhs/GSHHS_shp/c/GSHHS_c_L6.shp --test.on --test.path gshhs/GSHHS_shp/c/GSHHS_c_L6.shp --test.selector "rowid == 2"


./glgrib.x --test.on --test.selector "rowid == 6"  --coast.on --coast.lines.format shp  --coast.lines.path coastlines/shp/GSHHS_c_L1.shp --coast.lines.selector "rowid == 3" > shp1.eo 2>&1
./glgrib.x --test.on --test.selector ""            --coast.on --coast.lines.format shp  --coast.lines.path coastlines/shp/GSHHS_c_L1.shp --coast.lines.selector "rowid == 3" > shp2.eo 2>&1


