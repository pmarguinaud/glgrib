#!/bin/bash

set -x

make

./glGrib.x \
  --field[0].stream.motion.on \
  --field\[0\].path testdata/advection_850.grib%shortName=\"u\" testdata/advection_850.grib%shortName=\"v\" \
  --field\[0\].type STREAM --field\[0\].palette.colors blue red --land.on --grid.on --grid.resolution 18 \
  --grid.color black --landscape.on --landscape.path landscape/white.bmp --landscape.scale 0.99 \
  --view.lon -10.364 --view.lat 46.8244 --view.fov 5.3 --view.projection XYZ --window.width 1200 \
  --field[0].stream.width 0.5 --field[0].scale 1.001
