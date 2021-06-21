#!/bin/bash

/home/phi001/3d/glgrib/bin/glgrib \
  --field[0].scalar.discrete.on --field[0].path share/data/discrete/MYPROC.grb --field[0].scalar.discrete.missing_color pink --field[0].palette.colors '#00000000' \
  --field[0].scalar.widen.on --field[0].scalar.widen.value 2.0 --field[0].scalar.widen.size 30 \
  --field[1].scalar.discrete.on --field[1].path share/data/discrete/MYPROC.grb --field[1].scalar.discrete.missing_color blue --field[1].palette.colors '#00000000' \
  --grid.on --landscape.on --landscape.color black --landscape.scale 0.999 --view.lat 89
