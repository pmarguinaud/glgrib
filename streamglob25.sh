#!/bin/bash


tr=glob025


#/usr/bin/time -p \
#perf record \
gdb -ex run --args \
./glgrib.x \
  --field[0].palette.colors black green --field[0].palette.values 0. 40. --field[0].path testdata/wind+temp/$tr/S105WIND.U.PHYS.grb testdata/wind+temp/$tr/S105WIND.V.PHYS.grb --field[0].stream.on  \
  --grid.on --landscape.on --landscape.path landscape/white.bmp --field[0].scale 1.01 --grid.color blue --coast.on  --coast.lines.color pink \
  --field[1].vector.on --field[1].path testdata/wind+temp/$tr/S105WIND.U.PHYS.grb testdata/wind+temp/$tr/S105WIND.V.PHYS.grb --field[1].vector.color red --field[1].vector.scale 5 --field[1].scale 1.01  \
  --field[2].path testdata/wind+temp/$tr/S105WIND.U.PHYS.grb --field[2].scale 1.02  --field[2].palette.colors blue --field[2].scalar.wireframe.on \
  --view.lat 25.86 --view.lon -0.25 --view.fov 0.2


#--window.offscreen.on
#--field[2].path testdata/wind+temp/$tr/S105WIND.U.PHYS.grb --field[2].scalar.wireframe.on --field[2].scale 1.01 --field[2].palette.colors green green \
