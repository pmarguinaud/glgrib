## Contour in diff mode, global lat/lon geometry -- contour_diff
![](samples/contour_diff/TEST_0000.png)

```
    --field[0].diff.on --field[0].type CONTOUR --field[0].path 
    share/glgrib/testdata/glob01/lfpw_0_3_1_sfc_0_prmsl+0000.grib2 
    share/glgrib/testdata/glob01/lfpw_0_3_1_sfc_0_prmsl+0102.grib2 
    --field[0].scale 1.01 --landscape.on --landscape.color white 
    --field[0].user_pref.off 
```
## Display field difference -- scalar_diff
![](samples/scalar_diff/TEST_0000.png)

```
    --field[0].diff.on --field[0].path share/glgrib/testdata/interp/01.grb 
    share/glgrib/testdata/interp/36.grb --field[0].scale 1.01 --view.lon 2 
    --view.lat 46.7 --view.fov 5 --field[0].palette-{ --name cold_hot 
    --min -10 --max +10 }- 
```
