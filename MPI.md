## Display sphere partitionning (t1198)                              -- myproc
![](share/test/myproc/TEST_0000.png)

```
    --field[0].scalar.discrete.on --field[0].path 
    share/data/discrete/MYPROC.grb --field[0].palette.colors green 
    --field[0].scalar.discrete.missing_color black 
    --field[0].palette.colors #00000000 --field[1].path 
    share/data/discrete/SURFTEMPERATURE.grb --field[1]-{ --palette.max 
    313.15 --palette.min 253.15 --palette.name cold_hot_temp }- --view.lat 
    36 --view.lon -15 
```
## Display sphere partitionning (t1192c2.2)                          -- myproc22
![](share/test/myproc22/TEST_0000.png)

```
    --field[0].scalar.discrete.on --field[0].path 
    share/data/discrete_stretched/MYPROC.grb --field[0].palette.colors 
    green --field[0].scalar.discrete.missing_color black 
    --field[0].palette.colors #00000000 --field[1].path 
    share/data/discrete_stretched/SURFTEMPERATURE.grb --field[1]-{ 
    --palette.max 313.15 --palette.min 253.15 --palette.name cold_hot_temp 
    }- 
```
## MPI view                                                          -- mpiview
![](share/test/mpiview/TEST_0000.png)

```
    --field[0].path share/data/discrete/SURFTEMPERATURE.grb 
    --field[0].mpiview.on --field[0].mpiview.path 
    share/data/discrete/MYPROC.grb --field[0].mpiview.scale 0.2 --view.lon 
    31 --view.lat 41 
```
