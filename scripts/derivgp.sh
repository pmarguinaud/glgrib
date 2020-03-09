#!/bin/bash

id=$(cat id.txt)

VIEW="--view.lat 67.49 --view.lon 26.92 --view.fov 0.3"
VIEW="--view.lon 2 --view.lat 46.8  --view.fov 3.7"

./glGrib.x \
--field[0].path ftp://prolix.meteo.fr//scratch/work/marguina/tmp/tmp.$id/MYPROC.001.fa%TESTFIELD \
   --field[0].palette-{ --colors green }- \
   $VIEW --field[0].scalar.discrete.on --window.offscreen.format myproc1.png --window.offscreen.on

for d in 1 2 3
do
for k in "" 1
do

./glGrib.x $VIEW \
    --field[0].path ftp://prolix.meteo.fr//scratch/work/marguina/tmp/tmp.$id/ZFLDDY$k.00$d.fa%TESTFIELD --field[0].type contour  --field[0].palette.colors blue --field[0].contour-{ --min -0.25 --max 0.25 }- \
    --field[1].path ftp://prolix.meteo.fr//scratch/work/marguina/tmp/tmp.$id/ZFLDDX$k.00$d.fa%TESTFIELD --field[1].type contour  --field[1].palette.colors red  --field[1].contour-{ --min -0.25 --max 0.25 }- \
    --field[2].path ftp://prolix.meteo.fr//scratch/work/marguina/tmp/tmp.$id/ZFLD.00$d.fa%TESTFIELD --field[2].palette.name Greens --field[2].scale 0.999 --field[2].scalar.wireframe.on \
    --window.offscreen.on --window.offscreen.format gpderiv$k.$d.png --coast.on --coast.lines.color pink

done
done
