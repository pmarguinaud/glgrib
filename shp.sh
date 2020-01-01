#!/bin/bash

set -x

gdb -ex="set confirm on" -ex=run -ex=quit --args \
./glgrib.x --coast.on --coast.lines.format shp  --coast.lines.path coastlines/shp/GSHHS_c_L1.shp  --coast.lines.selector "rowid == 1" --grid.on --coast.lines.color red


