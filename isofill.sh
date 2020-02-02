#!/bin/bash


set -x

make 

./glgrib.x --field[0].path  --field\[0\].path testdata/aro2.5/S090WIND.U.PHYS.grb --field[0].type ISOFILL --field[0].palette-{ --colors  '#d8badbff' '#c187c4ff' '#ba56bcff' '#d821e0ff' '#f400ffff' '#f982ffff' }-  --view.lon 2.15 --view.lat 46.13 --view.fov 0.02


#--view.lon -5 --view.lat 52 --view.fov 5

