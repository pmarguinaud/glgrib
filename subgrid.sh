#!/bin/bash

./bin/glgrib --field[0].path ./share/data/t49/Z.grb  --landscape.on --landscape.path landscape/black.png --landscape.scale 0.99  --field[0].scalar.wireframe.on  --field[1].path ./share/data/t49/Z.grb --field[1].scalar.points.on --field[0].geometry.subgrid.on
