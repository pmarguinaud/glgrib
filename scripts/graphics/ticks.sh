#!/bin/bash

set -x

make 

./glgrib.x --landscape-{ --on --color black --scale 0.999 }- --grid-{ --on --labels.on --resolution 18 }-  --ticks-{ --on --font.scale 0.02 --format '%+.0f' }- --view.fov 7  --view.lat 50  --view.lon  0
