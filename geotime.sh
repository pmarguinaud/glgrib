#!/bin/bash

./bin/glgrib --geopoints.path share/data/geopoints/ssmis_light.nc --geopoints.on  --geopoints.points.palette.name cold_hot  --landscape.on  --geopoints.points.size.value 0.5 --grid.on --grid.color black --coast.on --coast.lines.color black --colorbar.on  --geopoints.points.palette.min 200 --geopoints.points.palette.max 240 --geopoints.lon col_4 --geopoints.lat col_3 --geopoints.val col_5 --landscape.scale 0.9
