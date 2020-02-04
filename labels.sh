#!/bin/bash

set -x

make 

./glgrib.x --window.width 1024 --window.height 1024 --landscape.on --field[0].path testdata/contour/latlon18x13.grb --field[0].scale 1.03 --field[0].type CONTOUR --view.lat 45.65 --view.lon 6.39 --field[0].contour.labels.on --grid.on --view.fov 10  --grid.color white --field[0].palette.colors red  --field[0].contour.labels-{ --font.color.foreground white --font.color.background black }- --coast.on --coast.lines.color pink

./glgrib.x --window.width 1024 --window.height 1024 --landscape.on --field[0].path testdata/z500.grb --field[0].scale 1.03 --field[0].type CONTOUR --view.lat 52 --view.lon 30 --field[0].contour.labels.on --grid.on --grid.color white --field[0].palette.colors red  --field[0].contour.labels-{ --font.color.foreground white --font.color.background black --format '%.0f' }- --coast.on --coast.lines.color pink
