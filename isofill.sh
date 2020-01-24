#!/bin/bash


set -x

make 

#gdb -ex=run --args \
./glgrib.x --field[0].path testdata/wind+temp/t0224/S105TEMPERATURE.grb  --field[0].type ISOFILL --field[0]-{ --palette.min 294 --palette.max 296 --user_pref.off --isofill.min 294 --isofill.max 296 --palette.name "cold_hot_temp" }-  --view.lat  31.76 --view.lon  24.58  --view.fov 0.4 --window.width 1200  --field[0].geometry.triangle_strip.off


