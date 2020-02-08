#!/bin/bash

set -x

make 

./glgrib.x --field[0].path testdata/t31c2.4/SURFIND.TERREMER.grb --field[0].palette.colors blue red
