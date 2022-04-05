#!/bin/bash


list="
SPEC.+0020.-0020.grb
SPEC.+0020.-0019.grb
SPEC.+0020.-0018.grb
SPEC.+0020.-0017.grb
SPEC.+0020.-0016.grb
SPEC.+0020.-0015.grb
SPEC.+0020.-0014.grb
SPEC.+0020.-0013.grb
SPEC.+0020.-0012.grb
SPEC.+0020.-0011.grb
SPEC.+0020.-0010.grb
SPEC.+0020.-0009.grb
SPEC.+0020.-0008.grb
SPEC.+0020.-0007.grb
SPEC.+0020.-0006.grb
SPEC.+0020.-0005.grb
SPEC.+0020.-0004.grb
SPEC.+0020.-0003.grb
SPEC.+0020.-0002.grb
SPEC.+0020.-0001.grb
SPEC.+0020.+0000.grb
SPEC.+0020.+0001.grb
SPEC.+0020.+0002.grb
SPEC.+0020.+0003.grb
SPEC.+0020.+0004.grb
SPEC.+0020.+0005.grb
SPEC.+0020.+0006.grb
SPEC.+0020.+0007.grb
SPEC.+0020.+0008.grb
SPEC.+0020.+0009.grb
SPEC.+0020.+0010.grb
SPEC.+0020.+0011.grb
SPEC.+0020.+0012.grb
SPEC.+0020.+0013.grb
SPEC.+0020.+0014.grb
SPEC.+0020.+0015.grb
SPEC.+0020.+0016.grb
SPEC.+0020.+0017.grb
SPEC.+0020.+0018.grb
SPEC.+0020.+0019.grb
SPEC.+0020.+0020.grb
"

set -x

prefix=$PWD


if [ 1 -eq 1 ]
then

rm -rf 1 
mkdir 1
cd 1

for f in $list
do

$prefix/bin/glgrib \
  --field[0].path $prefix/share/data/wind+temp/t1198/S105TEMPERATURE.grb \
  --field[1].path $prefix/share/data/wind+temp/t1198/spectral/$f --field[1].scale 1.01   \
  --field[1].type CONTOUR --field[1].palette.colors black white \
  --view.lon -25 --view.lat 25 --scene.light.on  --grid.on \
  --render.offscreen.on

done

cd ..

fi

if [ 0 -eq 1 ]
then

rm -rf 2
mkdir 2
cd 2


for f in $list
do

$prefix/bin/glgrib \
  --field[0].path $prefix/share/data/wind+temp/t1198/S105TEMPERATURE.grb  --field[0].geometry.height.on \
  --field[0].geometry.height.scale 0.2 --field[0].geometry.height.path $prefix/share/data/wind+temp/t1198/spectral/$f \
  --field[0].scale 0.8 --scene.light.on  --grid.on  --view.lon -25 --view.lat 25 \
  --render.offscreen.on

done

cd ..

fi

if [ 0 -eq 1 ]
then

rm -rf 3
mkdir 3
cd 3

for f in $list
do

$prefix/bin/glgrib \
  --field[0].path $prefix/share/data/wind+temp/t1198/S105TEMPERATURE.grb \
  --field[1].path $prefix/share/data/wind+temp/t1198/spectral/$f --field[1].scale 1.01   \
  --field[1].palette.colors '#00ff0044' '#00000100' '#00ff0044' --field[1].palette.values -1 0. +1 \
  --view.lon -25 --view.lat 25 --scene.light.on  --grid.on \
  --render.offscreen.on

done

cd ..

fi


if [ 0 -eq 1 ]
then

rm -rf 4
mkdir 4
cd 4

for f in $list
do

$prefix/bin/glgrib --field[0].path $prefix/share/data/wind+temp/t1198/S105TEMPERATURE.grb \
  --field[0].geometry.height.on --field[0].geometry.height.scale 0.2 --field[0].geometry.height.path $prefix/share/data/wind+temp/t1198/spectral/$f --field[0].scale 0.9 \
  --field[1].path $prefix/share/data/wind+temp/t1198/spectral/$f  --field[1].scale 0.91  --field[1].type CONTOUR --field[1].palette.colors black white  \
  --view.lon -25 --view.lat 25 --scene.light.on  --grid.on --field[1].geometry.height.on --field[1].geometry.height.scale 0.2 \
  --render.offscreen.on

done

cd ..

fi
