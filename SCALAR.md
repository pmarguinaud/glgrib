## Surface temperature, global lat/lon field -- glob01
![](blob/master/share/glgrib/test/glob01/TEST_0000.png?raw=true)

```
    --field[0].path share/glgrib/data/glob01/lfpw_0_0_0_sfc_0_t.grib2 
    --field[0].scale 1.00 --field[0].palette.name cold_hot_temp --coast.on 
    --grid.on 
```
## Lambert geometry, French Guyana -- guyane
![](blob/master/share/glgrib/test/guyane/TEST_0000.png?raw=true)

```
    --landscape.on --field[0].user_pref.off --field[0].path 
    share/glgrib/data/aro_guyane/SURFTEMPERATURE.grb 
    --field[0].palette.name cold_hot_temp --field[0].scale 1.01 --view.lat 
    5 --view.lon -51 --view.fov 3 --coast.on --grid.on 
```
## Scalar field on octahedral grid -- gauss_octahedral
![](blob/master/share/glgrib/test/gauss_octahedral/TEST_0000.png?raw=true)

```
    --field[0].path ./share/glgrib/data/ecmwf/ecmf_0_1_0_ml_137_q.grib2 
    --field[0].palette.colors gray cyan blue --field[0].palette.values 0 
    0.005 0.03 --colorbar.on --render.width 1200 
```
## SST on lat/lon grid -- sst
![](blob/master/share/glgrib/test/sst/TEST_0000.png?raw=true)

```
    --field[0].scale 1.001 --field[0].path share/glgrib/data/sst.grib 
    --field[0].palette-{ --values 271.15 273.15 275.15 277.15 279.15 
    281.15 283.15 285.15 287.15 289.15 291.15 293.15 295.15 297.15 299.15 
    301.15 303.15 305.15 307.15 309.15 --colors #4a007aff #9c00ffff 
    #cc78ffff #0000ffff #0059ffff #008cffff #0a7d00ff #0abf00ff #0aff00ff 
    #a19c00ff #dad300ff #ffed00ff #a85400ff #d66b00ff #ff8500ff #bf0a00ff 
    #ff0d00ff #ff857fff #ffd6d4ff --offset -273.15 }- --colorbar.on 
    --grid.on --landscape.on --landscape.path landscape/black.png 
    --view.lon -150 --view.projection LATLON --render.width 1500 
    --view.fov 13 --grid.color gray 
```
## Old ARPEGE -- oldarpege
![](blob/master/share/glgrib/test/oldarpege/TEST_0000.png?raw=true)

```
    --field[0].path share/glgrib/data/arpege.123.fa%SURFTEMPERATURE 
    --scene.date.on --scene.date.font.bitmap.scale 0.03 
```
## Simple shading -- shading
![](blob/master/share/glgrib/test/shading/TEST_0000.png?raw=true)

```
    --field[0].path share/glgrib/data/t850.grb --field[0].palette.values 
    245.15 297.15 --field[0].palette.generate.on 
    --field[0].palette.generate.levels 53 --field[0].palette.colors blue 
    red --colorbar.on --render.width 1200 --field[0].palette.rainbow.on 
    --field[0].palette.rainbow.direct.off --field[0].palette.offset 
    -273.15 --coast.on --coast.lines.color black --grid.on --grid.color 
    black --grid.labels.on --grid.labels.font.color.foreground black 
    --grid.labels.font.color.background white 
    --grid.labels.font.bitmap.scale 0.04 --render.width 1200 --view.lat 34 
    --view.lon 20 
```
## Rain, southern America -- southamerica
![](blob/master/share/glgrib/test/southamerica/TEST_0000.png?raw=true)

```
    --field[0].path share/glgrib/data/total_precipitation.grib 
    --field[0].palette-{ --values 0. 0.0005 0.001 0.002 0.005 0.010 0.020 
    0.050 0.100 0.200 0.250 --colors white #ffffd9 #edf8b1 #c7e9b4 #7fcdbb 
    #41b6c4 #1d91c0 #225ea8 #253494 #081d58 }- --coast.on 
    --coast.lines.color black --field[0].palette.linear.on 1000. 
    --colorbar.on --render.width 1200 --colorbar.font.color.foreground 
    black --view.projection LATLON --view.fov 4 --view.lat -20.1373 
    --view.lon -60.4499 --grid.on --grid.color black 
```
## Temperature with fixed palette -- fixedpaltemp
![](blob/master/share/glgrib/test/fixedpaltemp/TEST_0000.png?raw=true)

```
    --field[0].path share/glgrib/data/wind+temp/t0224/S105TEMPERATURE.grb 
    --field[0].user_pref.off --field[0].palette-{ --colors blue pink 
    orange yellow cyan --values 260 270 280 290 300 320 --fixed.on 
    --linear.on }- --field[0].type ISOFILL --field[0].isofill.levels 260 
    270 280 290 300 320 --colorbar.on --render.width 1200 --coast.on 
    --coast.lines.color black 
```
## Temperature with fixed palette (smooth) -- fixedpalscalsmooth
![](blob/master/share/glgrib/test/fixedpalscalsmooth/TEST_0000.png?raw=true)

```
    --field[0].path share/glgrib/data/wind+temp/t0224/S105TEMPERATURE.grb 
    --field[0].user_pref.off --field[0].palette-{ --colors blue pink 
    orange yellow cyan --values 260 270 280 290 300 320 --fixed.on 
    --linear.on }- --colorbar.on --render.width 1200 --coast.on 
    --coast.lines.color black --field[0].scalar.smooth.on 
```
## Temperature with fixed palette (rough) -- fixedpalscalrough
![](blob/master/share/glgrib/test/fixedpalscalrough/TEST_0000.png?raw=true)

```
    --field[0].path share/glgrib/data/wind+temp/t0224/S105TEMPERATURE.grb 
    --field[0].user_pref.off --field[0].palette-{ --colors blue pink 
    orange yellow cyan --values 260 270 280 290 300 320 --fixed.on 
    --linear.on }- --colorbar.on --render.width 1200 --coast.on 
    --coast.lines.color black 
```
## Display field in offscreen mode city lights -- citylight
![](blob/master/share/glgrib/test/citylight/TEST_0000.png?raw=true)

```
    --landscape.on --render.offscreen.on --render.offscreen.frames 36 
    --scene.light.rotate.on --scene.light.on --field[0].path 
    ./share/glgrib/data/town/town1800.grb --field[0].palette-{ --colors 
    #bbbb0000 #bbbb00ff #bbbb00ff --values 0. 0.1 1.0 }- 
    --field[0].scalar.light.reverse.on --field[1].path 
    share/glgrib/data/t1198c2.2/SURFNEBUL.BASSE.grb --field[1].scale 1.03 
    --field[1].palette.name cloud_auto --scene.light.rotate.rate 10 
```
