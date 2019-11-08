#!/bin/bash


tr=glob025
lev=SURF
lev=P020


#/usr/bin/time -p \
#perf record \
gdb -ex "set confirm on" -ex run -ex quit --args \
./glgrib.x \
  --field[0].path testdata/wind+temp/$tr/${lev}WIND.U.PHYS.grb testdata/wind+temp/$tr/${lev}WIND.V.PHYS.grb --field[0].stream.on --field[0].palette-{ --colors grey green --min 0. --max 40. --values 0. 40. }- --field[0].stream.width 1.0 \
  --field[1].vector.on --field[1].path testdata/wind+temp/$tr/${lev}WIND.U.PHYS.grb testdata/wind+temp/$tr/${lev}WIND.V.PHYS.grb --field[1].vector.color red --field[1].vector.scale 5 --field[1].scale 1.01  \
  --grid.on --landscape.on --landscape.path landscape/white.bmp --field[0].scale 1.01 --grid.color pink --coast.on  --coast.lines.color pink \
  --view.lat -3.87 --view.fov 0.2

exit
  --view.lat 89.25 --view.lon 87.28 --view.fov 0.1

  --field[2].path testdata/wind+temp/$tr/${lev}WIND.U.PHYS.grb --field[2].scale 1.02  --field[2].palette.colors blue --field[2].scalar.wireframe.on \

#--window.offscreen.on
#--field[2].path testdata/wind+temp/$tr/S105WIND.U.PHYS.grb --field[2].scalar.wireframe.on --field[2].scale 1.01 --field[2].palette.colors green green \
