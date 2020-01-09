#!/bin/bash

set -x

#/usr/bin/time -p perf record \
#./glgrib.x --coast.on --coast.lines.format shp  --coast.lines.path coastlines/shp/GSHHS_c_L1.shp  --coast.lines.selector "rowid == 1" --grid.on --test.on
#./glgrib.x --coast.on --coast.lines.format shp  --coast.lines.path gshhs/GSHHS_shp/c/GSHHS_c_L6.shp --test.on --test.path gshhs/GSHHS_shp/c/GSHHS_c_L6.shp --test.selector "rowid == 2"


 gdb -ex="set confirm on" -ex=run -ex=quit --args ./glgrib.x --test.on  --test.path coastlines/shp/GSHHS_i_L1.shp --test.selector "" --view.lat -43 --view.lon -65 --view.fov 2 --grid.on

exit

gdb -ex="set confirm on" -ex=run -ex=quit --args \
./glgrib.x --test.on --test.selector ""            --coast.on --coast.lines.format shp  --coast.lines.path coastlines/shp/GSHHS_c_L1.shp --coast.lines.selector "rowid == 426" 


