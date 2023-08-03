## TKE vertical profiles -- glob025verticaltke
![](samples/glob025verticaltke/TEST_0000.png)

```
    --landscape.on --colorbar.on --render.width 1200 --grid.on --view.fov 
    25 --field[0].type VERTICAL --field[0].vertical.lon 0. 30. 60. 90. 
    120. 150. 180. 210. 240. 270. 300. 330. 0. --field[0].vertical.lat +60 
    -60 +60 -60 +60 -60 +60 -60 +60 -60 +60 -60 +60 
    --field[0].vertical.height.constant.on 
    --field[0].vertical.height.constant.levels 0.002 0.05 0.1 0.15 0.20 
    0.25 0.30 --field[0].palette.min 0 --field[0].palette.max 5 
    --field[0].palette.colors #85c68855 #c1afcf #f2bc92 #fef896 #4777ae 
    --field[0].path share/glgrib/testdata/glob025/sfc_20_tke.grib2 
    share/glgrib/testdata/glob025/sfc_500_tke.grib2 
    share/glgrib/testdata/glob025/sfc_1000_tke.grib2 
    share/glgrib/testdata/glob025/sfc_1500_tke.grib2 
    share/glgrib/testdata/glob025/sfc_2000_tke.grib2 
    share/glgrib/testdata/glob025/sfc_2500_tke.grib2 
    share/glgrib/testdata/glob025/sfc_3000_tke.grib2 
```
