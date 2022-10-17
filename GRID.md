## Grid labels -- grid_labels
![](blob/master/share/glgrib/test/grid_labels/TEST_0000.png?raw=true)

```
    --grid.on --grid.labels.on --landscape.on 
```
## Display low & high -- hilo
![](blob/master/share/glgrib/test/hilo/TEST_0000.png?raw=true)

```
    --field[0].path share/glgrib/data/wind+temp/t0224/S105TEMPERATURE.grb 
    --field[0].hilo.on --field[0].hilo.font.color.foreground green 
```
## Grid ticks & labels -- tickslabels
![](blob/master/share/glgrib/test/tickslabels/TEST_0000.png?raw=true)

```
    --landscape-{ --on --scale 0.999 }- --grid-{ --on --resolution 18 }- 
    --ticks-{ --labels.on --labels.font.bitmap.scale 0.02 --labels.format 
    %.0f --lines.on --lines.color blue --lines.width 0.03 --lines.kind 1 
    }- --view.fov 7 --view.lat 50 --view.lon 0 
```
