#!/bin/bash

set -x

./glgrib.x --field[0].scale 1.001 --field[0].path 'metview/wind.grib%shortName="u"' 'metview/wind.grib%shortName="v"' --field[0].type vector --field[0].vector.hide_norm.on --field[0].vector.color black --field[1].path metview/vorticity.grib  --field[1].palette.colors $(cat list.colors.1)  --field[1].palette.values $(cat list.values.1)  --colorbar.on --coast.on --coast.lines.color black  --field[1].palette.linear.on
