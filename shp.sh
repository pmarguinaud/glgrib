#!/bin/bash

set -x

#/usr/bin/time -p perf record \

gdb -ex="set confirm on" -ex=run -ex=quit --args ./glgrib.x --land.on  --land.path coastlines/shp/GSHHS_c_L1.shp --land.selector "rowid <= 1000" \
	    --coast.on --coast.lines.path coastlines/shp/GSHHS_c_L1.shp --coast.lines.format shp --view.lon 0 --view.lat 0 

