#!/bin/bash


set -x

./glgrib.x \
  --field[0].scale 1.0005 \
  --field[0].path 'testdata/joachim_surf.grib%stepRange="18-24",shortName="10fg6"' \
  --field[0].palette.values 0 20 20.6 21.2 21.8 22.4 23 23.6 24.2 24.8 25.4 26 27.5 29 30.5 32 33.5 35 36.5 38 \
  --field[0].palette.colors '#00000000' '#88a8d6ff' '#819cd2ff' '#7a8fceff' '#7382caff' '#6c74c6ff' '#6666c2ff' '#6760bdff' '#6959b8ff' \
   '#6b53b4ff' '#6e4fadff' '#9045b8ff' '#bf3bc2ff' '#cc319dff' '#d62869ff' '#e11e28ff' '#eb4e14ff' '#f5960aff' '#ffea01ff' \
  --field[0].palette.linear.on --colorbar.on \
  --field[1].path 'testdata/joachim_surf.grib%stepRange=24,shortName="10u"' \
                  'testdata/joachim_surf.grib%stepRange=24,shortName="10v"' \
  --field[1].scale 1.001 --field[1].type vector --field[1].vector.norm.off --field[1].vector.arrow.color black \
  --field[1].vector.density 20 \
  --field[2].path 'testdata/joachim_surf.grib%stepRange=24,shortName="msl"' \
  --field[2].scale 1.001 --field[2].type contour --field[2].palette.colors black \
  --field[2].contour.levels 97000 97500 98000 98500 99000 99500 100000 100500 101000 101500 102000 \
  --field[2].contour.widths 3 3 3 3 1 1 1 1 1 1 3 \
  --window.width 1200 --grid.on --grid.color black --grid.resolution 18 \
  --coast.on --coast.lines.color black --coast.lines.path coastlines/gshhg/GSHHS_bin/gshhs_i.b \
  --border.on --border.lines.path coastlines/gshhg/WDBII_bin/wdb_borders_i.b \
  --view.clip.xmin 0.15 --view.projection POLAR_NORTH \
  --land.on --land.layers[0].color '#7c879cff' \
  --land.layers[0].path coastlines/shp/GSHHS_i_L1.shp \
  --land.layers[1].path coastlines/shp/GSHHS_i_L2.shp \
  --land.layers[2].path coastlines/shp/GSHHS_i_L3.shp \
  --land.layers[3].path coastlines/shp/GSHHS_i_L5.shp \
  --landscape.on --landscape.path "" --landscape.color white --landscape.scale 0.999 \
  --view.lon 4 --view.lat 51 --view.fov 3


