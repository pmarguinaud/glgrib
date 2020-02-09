#!/bin/bash

set -x

make 

#./glgrib.x --field[0].path testdata/t31c2.4/SURFIND.TERREMER.grb --field[0].palette.colors blue red

./glgrib.x --field[0].path testdata/aro2.5/SURFIND.TERREMER.grb --field[0].palette.name cold_hot --field[0].scale 1.00 --view.lat 46.2 --view.lon 2.0 --view.fov 5 --coast.on --grid.on
