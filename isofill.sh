#!/bin/bash


set -x

make 

./glgrib.x --field[0].path testdata/wind+temp/t0224/S105TEMPERATURE.grb --field[0].type ISOFILL --field[0]-{ --palette.name cold_hot_temp }- --window.width 1200 --view.lat 35.60 --view.lon 78.60 --view.fov 0.5 --field[0].geometry.triangle_strip.on
