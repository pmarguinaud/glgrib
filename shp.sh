#!/bin/bash

set -x

./glgrib.x --coast.on --coast.lines.format shp  --coast.lines.path coastlines/shp/GSHHS_c_L1.shp  --coast.lines.selector "rowid == 1" --grid.on


