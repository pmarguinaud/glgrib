## Streamlines on t1198c2.2 (surface) -- stream_t1198c22
![](blob/master/share/glgrib/test/stream_t1198c22/TEST_0000.png?raw=true)

```
    --field[0].scale 1.01 --field[0].path 
    share/glgrib/data/wind+temp/t1198c2.2/S105WIND.U.PHYS.grb 
    share/glgrib/data/wind+temp/t1198c2.2/S105WIND.V.PHYS.grb 
    --field[0].type STREAM --field[0].palette-{ --colors grey green --min 
    0. --max 40. --values 0. 40. }- --field[0].stream.width 1.0 
    --field[0].stream.density 1.0 --field[1].scale 1.01 --field[1].path 
    share/glgrib/data/wind+temp/t1198c2.2/S105WIND.U.PHYS.grb 
    share/glgrib/data/wind+temp/t1198c2.2/S105WIND.V.PHYS.grb 
    --field[1].type VECTOR --field[1].vector.norm.off 
    --field[1].vector.arrow.color red --grid.on --grid.color red 
    --grid.scale 1.02 --coast.lines.scale 1.02 --coast.on 
    --coast.lines.color red --landscape.on --landscape.path 
    landscape/white.png 
```
## Streamlines on GLOB025 (high level) -- stream_glob025
![](blob/master/share/glgrib/test/stream_glob025/TEST_0000.png?raw=true)

```
    --field[0].path 
    share/glgrib/data/wind+temp/glob025/P020WIND.U.PHYS.grb 
    share/glgrib/data/wind+temp/glob025/P020WIND.V.PHYS.grb 
    --field[0].type STREAM --field[0].palette-{ --colors grey green --min 
    0. --max 40. --values 0. 40. }- --field[0].stream.width 1.0 
    --field[1].type VECTOR --field[1].path 
    share/glgrib/data/wind+temp/glob025/P020WIND.U.PHYS.grb 
    share/glgrib/data/wind+temp/glob025/P020WIND.V.PHYS.grb 
    --field[1].vector.arrow.color red --field[1].vector.scale 5 
    --field[1].scale 1.01 --grid.on --landscape.on --landscape.path 
    landscape/white.bmp --field[0].scale 1.01 --grid.color pink --coast.on 
    --coast.lines.color pink 
```
## Streamlines on EURAT01 (surface) -- stream_eurat01
![](blob/master/share/glgrib/test/stream_eurat01/TEST_0000.png?raw=true)

```
    --field[0].path 
    share/glgrib/data/wind+temp/eurat01/H020WIND.U.PHYS.grb 
    share/glgrib/data/wind+temp/eurat01/H020WIND.V.PHYS.grb 
    --field[0].type STREAM --field[0].palette-{ --colors grey green --min 
    0. --max 40. --values 0. 40. }- --field[0].stream.width 1.0 
    --field[1].type VECTOR --field[1].path 
    share/glgrib/data/wind+temp/eurat01/H020WIND.U.PHYS.grb 
    share/glgrib/data/wind+temp/eurat01/H020WIND.V.PHYS.grb 
    --field[1].vector.arrow.color red --field[1].vector.scale 5 
    --field[1].scale 1.01 --grid.on --landscape.on --landscape.path 
    landscape/white.bmp --field[0].scale 1.01 --grid.color pink --coast.on 
    --coast.lines.color pink --view.lat 45 --view.lon 5 --view.fov 10 
```
## Streamlines on AROME 2.5km (surface) -- stream_lambert
![](blob/master/share/glgrib/test/stream_lambert/TEST_0000.png?raw=true)

```
    --field[0].path share/glgrib/data/aro2.5/S090WIND.U.PHYS.grb 
    share/glgrib/data/aro2.5/S090WIND.V.PHYS.grb --field[0].type STREAM 
    --field[0].palette-{ --colors grey green --min 0. --max 40. --values 
    0. 40. }- --field[0].stream.width 0.0 --field[0].stream.density 0.5 
    --field[0].stream.width 0.5 --field[1].type VECTOR --field[1].path 
    share/glgrib/data/aro2.5/S090WIND.U.PHYS.grb 
    share/glgrib/data/aro2.5/S090WIND.V.PHYS.grb 
    --field[1].vector.arrow.color red --field[1].vector.scale 1 
    --field[1].scale 1.01 --grid.on --landscape.on --landscape.path 
    landscape/white.bmp --field[0].scale 1.01 --grid.color pink --coast.on 
    --coast.lines.color pink --view.lon 2 --view.lat 46.7 --view.fov 4 
```
## Lat/lon streamlines -- latlonstream
![](blob/master/share/glgrib/test/latlonstream/TEST_0000.png?raw=true)

```
    --field[0].path share/glgrib/data/advection_850.grib%shortName="u" 
    share/glgrib/data/advection_850.grib%shortName="v" --field[0].type 
    STREAM --field[0].palette.colors darkblue --land.on 
    --land.layers[0].path share/glgrib/coastlines/shp/GSHHS_i_L1.shp 
    --land.layers[1].path share/glgrib/coastlines/shp/GSHHS_i_L2.shp 
    --land.layers[2].path share/glgrib/coastlines/shp/GSHHS_i_L3.shp 
    --land.layers[3].path share/glgrib/coastlines/shp/GSHHS_i_L5.shp 
    --land.layers[0].color grey --grid.on --grid.resolution 18 
    --grid.color black --landscape.on --landscape.path landscape/white.bmp 
    --landscape.scale 0.99 --view.lon -9.5 --view.lat 46 --view.fov 1.3 
    --view.projection LATLON --render.width 1200 
```
## Stream motion -- streammotion
![](blob/master/share/glgrib/test/streammotion/TEST_0000.png?raw=true)

```
    --field[0].stream.motion.on --field[0].path 
    share/glgrib/data/advection_850.grib%shortName="u" 
    share/glgrib/data/advection_850.grib%shortName="v" --field[0].type 
    STREAM --field[0].palette.name cold_hot --land.on --grid.on 
    --grid.resolution 18 --grid.color black --landscape.on 
    --landscape.path landscape/white.bmp --landscape.scale 0.99 --view.lon 
    -10.364 --view.lat 46.8244 --view.fov 5.3 --view.projection XYZ 
    --render.width 1200 --field[0].stream.width 0.5 --field[0].scale 1.001 
    --ticks.labels.on --ticks.labels.font.color.foreground black 
    --render.offscreen.on --render.offscreen.frames 36 
```
