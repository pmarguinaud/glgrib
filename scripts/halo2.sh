#!/bin/bash

/home/phi001/3d/glgrib/bin/glgrib \
  --field[0].scalar.widen.on --field[0].scalar.widen.value 18.0 --field[0].scalar.widen.size 20 --field\[0\].scalar.discrete.on \
  --field\[0\].path share/data/discrete/MYPROC.grb --field\[0\].palette.colors green --field\[0\].scalar.discrete.missing_color black \
  --field\[0\].palette.colors \#00000000 --field\[1\].path share/data/discrete/SURFTEMPERATURE.grb --field\[1\]-\{ --palette.max 313.15 --palette.min 253.15 \
  --palette.name cold_hot_temp \}- --view.lat 36 --view.lon -15
