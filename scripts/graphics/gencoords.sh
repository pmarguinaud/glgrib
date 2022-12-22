#!/bin/bash

set -x

make 

./bin/glgrib \
  --field[0].path share/data/wind+temp/t0224/S105TEMPERATURE.grb  \
  --field[0].geometry.gencoords.on --field[0].geometry.gaussian.fit.on

./bin/glgrib \
  --field[0].path share/data/aro2.5/SURFIND.TERREMER.grb --field[0].palette.name cold_hot \
  --field[0].scale 1.00 --view.lat 46.2 --view.lon 2.0 --view.fov 5 --coast.on --grid.on
