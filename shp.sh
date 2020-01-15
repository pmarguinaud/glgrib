#!/bin/bash

set -x

#/usr/bin/time -p perf record \
#./glgrib.x --coast.on --coast.lines.format shp  --coast.lines.path coastlines/shp/GSHHS_c_L1.shp  --coast.lines.selector "rowid == 1" --grid.on --test.on
#./glgrib.x --coast.on --coast.lines.format shp  --coast.lines.path gshhs/GSHHS_shp/c/GSHHS_c_L6.shp --test.on --test.path gshhs/GSHHS_shp/c/GSHHS_c_L6.shp --test.selector "rowid == 2"


gdb -ex="set confirm on" -ex=run -ex=quit --args ./glgrib.x --land.on  --land.path coastlines/shp/GSHHS_c_L1.shp --land.selector "rowid <= 10" \
	    --coast.on --coast.lines.path coastlines/shp/GSHHS_c_L1.shp --coast.lines.format shp --view.lon 39 --view.lat -12 --view.fov 3

exit

gdb -ex="set confirm on" -ex=run -ex=quit --args \
./glgrib.x --test.on --test.selector ""            --coast.on --coast.lines.format shp  --coast.lines.path coastlines/shp/GSHHS_c_L1.shp --coast.lines.selector "rowid == 426" 


