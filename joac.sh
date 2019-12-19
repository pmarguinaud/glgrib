#!/bin/bash


set -x

./glgrib.x \
  --field[0].scale 1.0005 \
  --field[0].path '/home/phi001/tmp/joachim_surf.grib%stepRange="18-24",shortName="10fg6"' \
  --field[0].palette.values $(cat ~/tmp/joac.values) --field[0].palette.colors $(cat ~/tmp/joac.colors) \
  --field[0].palette.linear.on --colorbar.on \
  --field[1].path '/home/phi001/tmp/joachim_surf.grib%stepRange=24,shortName="10u"' \
                  '/home/phi001/tmp/joachim_surf.grib%stepRange=24,shortName="10v"' \
  --field[1].scale 1.001 --field[1].type vector --field[1].vector.hide_norm.on --field[1].vector.color black \
  --field[1].vector.density 20 \
  --field[2].path '/home/phi001/tmp/joachim_surf.grib%stepRange=24,shortName="msl"' \
  --field[2].scale 1.001 --field[2].type contour --field[2].palette.colors black \
  --field[2].contour.levels 97000 97500 98000 98500 99000 99500 100000 100500 101000 101500 102000 \
  --field[2].contour.widths 3 3 3 3 1 1 1 1 1 1 3 \
  --window.width 1200 --grid.on --grid.color black --grid.resolution 18 \
  --coast.on --coast.lines.color green \
  --landscape.on --landscape.path landscape/white.png --view.clip.xmin 0.15 --view.projection POLAR_NORTH \
  --view.lon 4 --view.lat 51 --view.fov 3
