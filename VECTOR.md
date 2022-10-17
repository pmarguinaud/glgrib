## Wind Australia -- windaustralia3
![](tree/master/share/glgrib/test/windaustralia3/TEST_0000.png)

```
    --field[0].path share/glgrib/data/uv200.grib%paramId=131 
    share/glgrib/data/uv200.grib%paramId=132 --field[0].type VECTOR 
    --coast.on --view.lat -37 --view.lon 140 --view.fov 3 
    --field[0].vector.norm.off --land.on --landscape.on --landscape.color 
    #333333 --landscape.scale 0.999 --coast.on --coast.lines.color black 
    --coast.lines.path share/glgrib/coastlines/gshhg/GSHHS_bin/gshhs_i.b 
    --field[0].vector.scale 1 --render.width 1200 --field[0].scale 1.001 
    --view.projection LATLON --field[0].vector.density 10 
    --field[0].palette.colors #1cb8a6 #19c25c #24cb15 #80d511 #e0d50d 
    #ea7209 #f50408 #ff007f --field[0].palette.values 20 30 40 50 60 70 80 
    90 100 --field[0].vector.arrow.color #00000000 
    --field[0].vector.arrow.fixed.on --field[0].vector.arrow.min 10 
    --land.layers[0].color black --coast.on --coast.lines.color #555555 
    --colorbar.on --field[0].vector.density 20 --field[0].vector.scale 2 
```
## Wind on small AROME domain (raster & vector) -- small_aro
![](tree/master/share/glgrib/test/small_aro/TEST_0000.png)

```
    --field[0].type VECTOR --field[0].path 
    share/glgrib/data/aro_small/S041WIND.U.PHYS.grb 
    share/glgrib/data/aro_small/S041WIND.V.PHYS.grb --field[0].scale 1.00 
    --view.lon 26.64 --view.lat 67.36 --view.fov 0.5 --coast.on --grid.on 
```
## Display vector norm & arrow, Lambert geometry -- vector
![](tree/master/share/glgrib/test/vector/TEST_0000.png)

```
    --field[0].type VECTOR --field[0].path 
    share/glgrib/data/aro2.5/S090WIND.U.PHYS.grb 
    share/glgrib/data/aro2.5/S090WIND.V.PHYS.grb --field[0].scale 1.00 
    --field[0].vector.arrow.color green --view.lat 46.2 --view.lon 2.0 
    --view.fov 5 --coast.on --grid.on 
```
## Display vector norm, global lat/lon geometry -- vector_glob25
![](tree/master/share/glgrib/test/vector_glob25/TEST_0000.png)

```
    --landscape.on --landscape.grid.path 
    share/glgrib/data/arpt1798_wind/lfpw_0_2_2_sfc_20_u.grib2 
    --field[0].type VECTOR --field[0].path 
    share/glgrib/data/arpt1798_wind/glob25_+1.grb 
    share/glgrib/data/arpt1798_wind/glob25_+1.grb --field[0].scale 1.01 
    --coast.on --grid.on 
```
## Vector arrow, stretched/rotated gaussian geometry -- vector_t1798
![](tree/master/share/glgrib/test/vector_t1798/TEST_0000.png)

```
    --landscape.on --landscape.grid.path 
    share/glgrib/data/arpt1798_wind/+1.grb --field[0].type VECTOR 
    --field[0].path share/glgrib/data/arpt1798_wind/+1.grb 
    share/glgrib/data/arpt1798_wind/+1.grb --field[0].scale 1.01 
    --coast.on --grid.on 
```
## Display wind on stretched/rotated gaussian geometry -- wind_arp
![](tree/master/share/glgrib/test/wind_arp/TEST_0000.png)

```
    --landscape.on --landscape.grid.path share/glgrib/data/t31c2.4/Z.grb 
    --field[0].type VECTOR --field[0].path 
    share/glgrib/data/t31c2.4/S015WIND.U.PHYS.grb 
    share/glgrib/data/t31c2.4/S015WIND.V.PHYS.grb --field[0].scale 1.01 
    --coast.on --grid.on 
```
## Wind on global lat/lon geometry -- wind_glob25
![](tree/master/share/glgrib/test/wind_glob25/TEST_0000.png)

```
    --landscape.on --landscape.grid.path 
    share/glgrib/data/arpt1798_wind/lfpw_0_2_2_sfc_20_u.grib2 
    --field[0].type VECTOR --field[0].path 
    share/glgrib/data/arpt1798_wind/lfpw_0_2_2_sfc_20_u.grib2 
    share/glgrib/data/arpt1798_wind/lfpw_0_2_3_sfc_20_v.grib2 
    --field[0].scale 1.01 --coast.on --grid.on 
```
## Wind on stretched/rotated gaussian geometry -- wind_t1798
![](tree/master/share/glgrib/test/wind_t1798/TEST_0000.png)

```
    --landscape.on --landscape.grid.path 
    share/glgrib/data/arpt1798_wind/S105WIND.U.PHYS.grb --field[0].type 
    VECTOR --field[0].path 
    share/glgrib/data/arpt1798_wind/S105WIND.U.PHYS.grb 
    share/glgrib/data/arpt1798_wind/S105WIND.V.PHYS.grb --field[0].scale 
    1.01 --coast.on --grid.on 
```
## Wind & vorticity -- wind4
![](tree/master/share/glgrib/test/wind4/TEST_0000.png)

```
    --field[0].path share/glgrib/data/wind4/vorticity.grib 
    --field[0].palette.colors #00004cff #00007fff #0000b2ff #0000e5ff 
    #0026ffff #004cffff #0072ffff #0099ffff #00bfffff #00d800ff #33f2ffff 
    #72ffffff #bfffffff #ffffffff #ffff00ff #ffe500ff #ffcc00ff #ffb200ff 
    #ff9900ff #ff7f00ff #ff6600ff #ff4c00ff #ff2600ff #e50000ff #b20000ff 
    #7f0000ff #4c0000ff --field[0].palette.values -0.002 -0.001 -0.00075 
    -0.0005 -0.0003 -0.0002 -0.00015 -0.00013 -0.00011 -9e-05 -7e-05 
    -5e-05 -3e-05 -1e-05 1e-05 3e-05 5e-05 7e-05 9e-05 0.00011 0.00013 
    0.00015 0.0002 0.0003 0.0005 0.00075 0.001 0.002 
    --field[0].palette.linear.on --field[0].palette.scale 100000 
    --field[1].scale 1.001 --field[1].path 
    share/glgrib/data/wind4/wind.grib%shortName="u" 
    share/glgrib/data/wind4/wind.grib%shortName="v" --field[1].type vector 
    --field[1].vector.norm.off --field[1].vector.arrow.color black 
    --colorbar.on --coast.on --coast.lines.color black --coast.on 
    --coast.lines.color black --grid.on --grid.color black --view.lat 50 
    --view.fov 3 --view.projection POLAR_NORTH --view.clip.xmin 0.15 
    --render.width 1500 --grid.resolution 18 
```
## Wind on global lat/lon grid -- windlatlon
![](tree/master/share/glgrib/test/windlatlon/TEST_0000.png)

```
    --field[0].path share/glgrib/data/data_uv.grib%shortName="u" 
    share/glgrib/data/data_uv.grib%shortName="v" --field[0].type vector 
    --field[0].vector.arrow.off --field[0].palette.values 0 10 15 20 25 30 
    40 50 60 80 100 --colorbar.on --field[0].palette.linear.on 
    --field[0].palette-{ --min 0 --max 100 }- --field[0].palette.colors 
    #ffffffff #ffff66ff #daff00ff #94ff00ff #6ca631ff #00734bff #005447ff 
    #004247ff #003370ff #0033a3ff --coast.on --coast.lines.color black 
    --grid.on --grid.color black --grid.resolution 18 --view.fov 10 
    --view.projection LATLON --render.width 1650 --render.height 750 
```
## Wind Australia -- windaustralia
![](tree/master/share/glgrib/test/windaustralia/TEST_0000.png)

```
    --field[0].path share/glgrib/data/uv200.grib%paramId=131 
    share/glgrib/data/uv200.grib%paramId=132 --field[0].type VECTOR 
    --coast.on --view.lat -25 --view.lon 140 --view.fov 8 
    --field[0].vector.norm.off --land.on --field[0].vector.arrow.color 
    blue --landscape.on --landscape.color white --landscape.scale 0.999 
    --coast.on --coast.lines.color black --coast.lines.path 
    share/glgrib/coastlines/gshhg/GSHHS_bin/gshhs_i.b 
    --field[0].vector.density 100 --field[0].vector.scale 10 
    --render.width 1200 --field[0].scale 1.001 
```
## Wind Australia -- windaustralia2
![](tree/master/share/glgrib/test/windaustralia2/TEST_0000.png)

```
    --field[0].path share/glgrib/data/uv200.grib%paramId=131 
    share/glgrib/data/uv200.grib%paramId=132 --field[0].type VECTOR 
    --coast.on --view.lat -25 --view.lon 140 --view.fov 2 
    --field[0].vector.norm.off --land.on --field[0].vector.arrow.color 
    darkgreen --landscape.on --landscape.color white --landscape.scale 
    0.999 --coast.on --coast.lines.color black --coast.lines.path 
    share/glgrib/coastlines/gshhg/GSHHS_bin/gshhs_i.b 
    --field[0].vector.scale 5 --render.width 1200 --field[0].scale 1.001 
    --view.projection LATLON --field[0].vector.density 10 
```
## Wind Australia -- windaustralia4
![](tree/master/share/glgrib/test/windaustralia4/TEST_0000.png)

```
    --field[0].path share/glgrib/data/uv200.grib%paramId=131 
    share/glgrib/data/uv200.grib%paramId=132 --field[0].type VECTOR 
    --coast.on --view.lat -25 --view.lon 140 --view.fov 8 
    --field[0].vector.norm.off --land.on --landscape.on --landscape.color 
    white --landscape.scale 0.999 --coast.on --coast.lines.color black 
    --coast.lines.path share/glgrib/coastlines/gshhg/GSHHS_bin/gshhs_i.b 
    --field[0].vector.density 20 --field[0].vector.scale 0.5 
    --render.width 1200 --field[0].scale 1.001 --field[0].vector.arrow.off 
    --field[0].vector.barb.on --field[0].vector.barb.color brown 
    --field[0].vector.barb.circle.level 15 
```
## Wind + geopotential -- windgeop
![](tree/master/share/glgrib/test/windgeop/TEST_0000.png)

```
    --field[0].palette.colors black --field[0].contour.widths 1 2 1 1 2 1 
    1 2 1 1 2 --field[0].contour.levels 89000 89500 90000 90500 91000 
    91500 92000 92500 93000 93500 94000 94500 95000 95500 96000 96500 
    97000 97500 98000 98500 99000 99500 --field[0].path 
    share/glgrib/data/ghtuv.grib%shortName="z" --field[0].type CONTOUR 
    --land.on --landscape.on --landscape.color white --landscape.scale 
    0.999 --field[1]-{ --type vector --path 
    share/glgrib/data/ghtuv.grib%shortName="u" 
    share/glgrib/data/ghtuv.grib%shortName="v" --vector-{ --norm.off 
    --arrow.off --barb.on --barb.color blue --density 20 --scale 0.4 }- 
    --scale 1.001 }- --render.width 1200 --view.lat 46.2 --view.lon -10 
    --view.fov 5 --view.projection POLAR_NORTH --grid.on --grid.color 
    black --grid.resolution 18 
```
