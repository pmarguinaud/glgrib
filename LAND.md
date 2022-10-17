## Display land surface -- land
![](blob/master/share/glgrib/test/land/TEST_0000.png?raw=true)

```
    --land.on 
```
## Display land surface (high resolution) -- land_high
![](blob/master/share/glgrib/test/land_high/TEST_0000.png?raw=true)

```
    --land.on --land.layers[0].path 
    share/glgrib/coastlines/shp/GSHHS_h_L1.shp --land.layers[1].path 
    share/glgrib/coastlines/shp/GSHHS_h_L2.shp --land.layers[2].path 
    share/glgrib/coastlines/shp/GSHHS_h_L3.shp --land.layers[3].path 
    share/glgrib/coastlines/shp/GSHHS_h_L5.shp --view.lon 7 --view.lat 60 
    --view.fov 2 
```
## Coastlines -- coastlines
![](blob/master/share/glgrib/test/coastlines/TEST_0000.png?raw=true)

```
    --landscape.on --landscape.scale 0.999 --land.on --landscape.color 
    white --rivers.on --rivers.lines.color blue --border.on 
    --border.lines.color red --coast.on --coast.lines.color black 
    --coast.lines.path share/glgrib/coastlines/gshhg/GSHHS_bin/gshhs_i.b 
    --land.layers[0].path share/glgrib/coastlines/shp/GSHHS_i_L1.shp 
    --land.layers[1].path share/glgrib/coastlines/shp/GSHHS_i_L2.shp 
    --view.fov 5 --view.lat 44.5394 --view.lon 27.5924 --render.width 1200 
```
