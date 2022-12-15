## Cities -- cities
![](samples/cities/TEST_0000.png)

```
    --landscape.on --cities.on --cities.points.scale 1.01 
    --cities.points.size.value 1 --cities.points.size.variable.on 
    --view.lat 46.7 --view.lon 2 --view.fov 2 --cities.points.size.value 2 
    --cities.labels.on --cities.labels.font.color.foreground red 
    --cities.labels.font.bitmap.scale 0.04 
```
## Enable debug mode -- debug
![](samples/debug/TEST_0000.png)

```
    --landscape.on --landscape.grid.path share/glgrib/data/t1198c2.2/Z.grb 
    --field[0].path share/glgrib/data/t1198c2.2/N.grb --field[0].scale 
    1.03 --render.debug.on 
```
## Colorbar -- colorbar
![](samples/colorbar/TEST_0000.png)

```
    --landscape.on --landscape.grid.path share/glgrib/data/t1198c2.2/Z.grb 
    --field[0].path share/glgrib/data/t1198c2.2/N.grb --field[0].scale 
    1.03 --colorbar.on --colorbar.font.color.foreground green 
    --render.width 1000 
```
## Display French departements -- departements
![](samples/departements/TEST_0000.png)

```
    --departements.on --departements.lines.color blue 
    --departements.lines.selector 
    code_insee = 23 or code_insee = 19 or code_insee = 87 --view.lon 2 
    --view.lat 46.7 --view.fov 5 --coast.on 
```
## Test FA format, display sea fraction -- fa
![](samples/fa/TEST_0000.png)

```
    --field[0].path share/glgrib/data/fa/PGD.t479.fa%SFX.FRAC_SEA 
    --field[0].scale 1.03 
```
## Image on top of cloud fields -- image
![](samples/image/TEST_0000.png)

```
    --landscape.on --landscape.grid.path share/glgrib/data/t1198c2.2/Z.grb 
    --field[0].path share/glgrib/data/t1198c2.2/N.grb --field[0].scale 
    1.03 --scene.image.on --scene.image.path share/glgrib/data/image/B.bmp 
    --scene.image.x0 0. --scene.image.y0 0. --scene.image.x1 0.1 
    --scene.image.y1 0.1 
```
## Interpolate field over time -- interpolation
![](samples/interpolation/TEST.gif)

```
    --field[0].path share/glgrib/data/interp/01.grb 
    share/glgrib/data/interp/36.grb --field[0].scale 1.03 
    --field[0].palette.name cold_hot_temp --view.lon 2 --view.lat 46.7 
    --view.fov 5 --scene.interpolation.on --scene.interpolation.frames 200 
    --render.width 1000 --render.height 1000 
    --scene.date.font.bitmap.scale 0.03 --scene.date.font.color.foreground 
    red --scene.date.on --coast.on --grid.on --render.offscreen.on 
    --render.offscreen.frames 72 
```
## Display mapscale -- mapscale
![](samples/mapscale/TEST_0000.png)

```
    --landscape.on --mapscale.on 
```
## Display field with missing values -- missingvalue
![](samples/missingvalue/TEST_0000.png)

```
    --field[0].path share/glgrib/data/t49/SFX.CLAY.grb --coast.on 
    --grid.on --field[0].user_pref.off 
```
## Display field with keeping field values in RAM -- novalue
![](samples/novalue/TEST_0000.png)

```
    --landscape.on --landscape.grid.path share/glgrib/data/t1798/Z.grb 
    --field[0].path share/glgrib/data/t1798/SURFNEBUL.BASSE.grb 
    --field[0].scale 1.03 --field[0].palette.name cloud_auto 
    --field[0].no_value_pointer.on 
```
## Display field in offscreen mode -- offscreen
![](samples/offscreen/TEST.gif)

```
    --landscape.on --landscape.grid.path share/glgrib/data/t1198c2.2/Z.grb 
    --render.offscreen.on --render.offscreen.frames 36 
    --scene.light.rotate.on --scene.light.on --field[0].path 
    share/glgrib/data/t1198c2.2/SURFNEBUL.BASSE.grb 
    share/glgrib/data/t1198c2.2/SURFNEBUL.MOYENN.grb 
    share/glgrib/data/t1198c2.2/SURFNEBUL.HAUTE.grb --field[0].scale 1.03 
    1.03 1.03 --field[0].palette.name cloud_auto cloud_auto cloud_auto 
    --scene.light.rotate.rate 10 
```
## Display field with options in file -- optionsfile
![](samples/optionsfile/TEST_0000.png)

```
    --{share/glgrib/data/options.list} 
```
## Select field from GRIB file -- select_grib
![](samples/select_grib/TEST_0000.png)

```
    --landscape.on --landscape.grid.path share/glgrib/data/t1198c2.2/Z.grb 
    --field[0].path 
    share/glgrib/data/t1198c2.2/N.grb%parameterCategory=6,year=2019 
    --field[0].scale 1.03 
```
## Low-res ARPEGE resolution -- small
![](samples/small/TEST_0000.png)

```
    --landscape.on --landscape.grid.path share/glgrib/data/t49/Z.grb 
    --field[0].scale 1.01 --field[0].path 
    share/glgrib/data/t49/SFX.CLAY.grb --coast.on --grid.on 
    --field[0].user_pref.off 
```
## Test travelling option -- travelling
![](samples/travelling/TEST.gif)

```
    --scene.travelling.on --scene.travelling.pos1.lon 0 
    --scene.travelling.pos1.lat 0 --scene.travelling.pos1.fov 30 
    --scene.travelling.pos2.lon 90 --scene.travelling.pos2.lat 45 
    --scene.travelling.pos2.fov 10 --landscape.on --landscape.grid.path 
    share/glgrib/data/t1198c2.2/Z.grb --field[0].path 
    share/glgrib/data/t1198c2.2/N.grb --field[0].scale 1.03 
    --render.offscreen.on --render.offscreen.frames 72 
```
## Zoom with Schmidt transform -- zoomschmidt1
![](samples/zoomschmidt1/TEST_0000.png)

```
    --field[0].path share/glgrib/data/wind+temp/t1798/S105TEMPERATURE.grb 
    --view.zoom.on --coast.on --coast.lines.color black --view.lon 2 
    --view.lat 46.7 --view.zoom.stretch 0.2 --grid.off 
```
