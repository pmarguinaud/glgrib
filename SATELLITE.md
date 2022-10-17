## SSMI satellite data -- ssmigeopoints
![](tree/master/share/glgrib/test/ssmigeopoints/TEST_0000.png)

```
    --geopoints.path share/glgrib/data/geopoints/ssmis_light.nc 
    --geopoints.on --geopoints.points.scale 1.005 
    --geopoints.points.palette.name cold_hot --landscape.on 
    --geopoints.points.size.value 0.5 --grid.on --grid.color black 
    --coast.on --coast.lines.color black --colorbar.on 
    --geopoints.points.palette.min 200 --geopoints.points.palette.max 240 
    --geopoints.lon col_4 --geopoints.lat col_3 --geopoints.val col_5 
    --render.width 1200 --view.lat -32.736 --view.lon 41.909 
```
## SSMI satellite data (polar projection) -- ssmigeopointspolar
![](tree/master/share/glgrib/test/ssmigeopointspolar/TEST_0000.png)

```
    --geopoints.path share/glgrib/data/geopoints/ssmis_light.nc 
    --geopoints.on --geopoints.points.palette.name cold_hot --landscape.on 
    --geopoints.points.size.value 0.5 --grid.on --grid.color black 
    --coast.on --coast.lines.color black --colorbar.on 
    --geopoints.points.palette.min 200 --geopoints.points.palette.max 240 
    --geopoints.lon col_4 --geopoints.lat col_3 --geopoints.val col_5 
    --render.width 1200 --view.lat 62.577 --view.lon 71.4523 
    --view.projection POLAR_NORTH 
```
## SSMI satellite data (lat/lon) -- ssmigeopointlatlon
![](tree/master/share/glgrib/test/ssmigeopointlatlon/TEST_0000.png)

```
    --geopoints.path share/glgrib/data/geopoints/ssmis_light.nc 
    --geopoints.on --geopoints.points.palette.name cold_hot --landscape.on 
    --geopoints.points.size.value 0.5 --grid.on --grid.color black 
    --coast.on --coast.lines.color black --colorbar.on 
    --geopoints.points.palette.min 200 --geopoints.points.palette.max 240 
    --geopoints.lon col_4 --geopoints.lat col_3 --geopoints.val col_5 
    --render.width 1200 --view.fov 10 --view.lat 5.04296 --view.lon 
    -9.39295 --view.projection LATLON --view.clip.xmin 0.15 
```
