#!/bin/bash

set -x

make 

./glgrib.x --window.width 1024 --window.height 1024 --landscape.on --landscape.path landscape/black.bmp --field\[0\].path testdata/contour/latlon18x13.grb --field\[0\].scale 1.03 --field\[0\].type CONTOUR --view.lat 45 --view.lon 7  --field[0].contour.labels.on
