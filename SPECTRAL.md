## Spherical harmonic (T1198) -- harmonic
![](blob/master/share/glgrib/test/harmonic/TEST_0000.png?raw=true)

```
    --field[0].path share/glgrib/data/harmonics/SPEC.+0008.+0004.grb 
    --field[0].palette-{ --colors blue green red --values -1 0. 1 }- 
    --field[0].geometry.height.on --field[0].geometry.height.scale 0.2 
    --view.fov 25 --view.lon 20 --scene.light.on 
```
## Spherical harmonic (T1198C2.2) -- harmonic_stretch
![](blob/master/share/glgrib/test/harmonic_stretch/TEST_0000.png?raw=true)

```
    --field[0].path 
    share/glgrib/data/harmonics/SPEC.+0008.+0004_stretched.grb 
    --field[0].palette-{ --colors blue green red --values -1 0. 1 }- 
    --field[0].geometry.height.on --field[0].geometry.height.scale 0.2 
    --view.fov 25 --view.lon 20 --scene.light.on 
```
## Spherical harmonic (T1198) -- tharmonic1
![](blob/master/share/glgrib/test/tharmonic1/TEST_0000.png?raw=true)

```
    --field[0].path share/glgrib/data/wind+temp/t1198/S105TEMPERATURE.grb 
    --field[1].path share/glgrib/data/harmonics/SPEC.+0008.+0004.grb 
    --field[1].scale 1.01 --field[1].type CONTOUR 
    --field[1].palette.colors black white --view.lon -25 --view.lat 25 
    --scene.light.on --grid.on 
```
## Spherical harmonic (T1198) -- tharmonic2
![](blob/master/share/glgrib/test/tharmonic2/TEST_0000.png?raw=true)

```
    --field[0].path share/glgrib/data/wind+temp/t1198/S105TEMPERATURE.grb 
    --field[0].geometry.height.on --field[0].geometry.height.scale 0.2 
    --field[0].geometry.height.path 
    share/glgrib/data/harmonics/SPEC.+0008.+0004.grb --field[0].scale 0.8 
    --scene.light.on --grid.on --view.lon -25 --view.lat 25 
```
## Spherical harmonic (T1198) -- tharmonic3
![](blob/master/share/glgrib/test/tharmonic3/TEST_0000.png?raw=true)

```
    --field[0].path share/glgrib/data/wind+temp/t1198/S105TEMPERATURE.grb 
    --field[1].path share/glgrib/data/harmonics/SPEC.+0008.+0004.grb 
    --field[1].scale 1.01 --field[1].palette.colors #00ff0044 #00000100 
    #00ff0044 --field[1].palette.values -1 0. +1 --view.lon -25 --view.lat 
    25 --scene.light.on --grid.on 
```
## Spherical harmonic (T1198) -- tharmonic4
![](blob/master/share/glgrib/test/tharmonic4/TEST_0000.png?raw=true)

```
    --field[0].path share/glgrib/data/wind+temp/t1198/S105TEMPERATURE.grb 
    --field[0].geometry.height.on --field[0].geometry.height.scale 0.2 
    --field[0].geometry.height.path 
    share/glgrib/data/wind+temp/t1198/spectral/SPEC.+0020.-0020.grb 
    --field[0].scale 0.8 --scene.light.on --grid.on --view.lon -25 
    --view.lat 25 
```
