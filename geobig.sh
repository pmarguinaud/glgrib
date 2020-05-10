#!/bin/bash

./bin/glgrib --geopoints.path ~/tmp/ssmis.nc --geopoints.on  --geopoints.points.palette.name cold_hot  --landscape.on  --geopoints.points.size.value 0.5 --grid.on --grid.color black --coast.on --coast.lines.color black --colorbar.on  --geopoints.points.palette.min 200 --geopoints.points.palette.max 240 --geopoints.lon col_7 --geopoints.lat col_6 --geopoints.val col_8
