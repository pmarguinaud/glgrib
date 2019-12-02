#!/bin/bash

set -x

make && ./glgrib.x --field\[0\].path testdata/contour/latlon9x6.grb --field\[0\].scale 1.03 --field\[0\].type CONTOUR --field[1].path testdata/contour/latlon9x6.grb --field[1].scalar.wireframe.on  --field[1].palette.colors red  --field[0].palette.colors green --view.lat 46 --view.projection LATLON  --view.fov 5


