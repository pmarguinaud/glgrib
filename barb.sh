#!/bin/bash

set -x

make


./glgrib.x --field[0].path testdata/aro2.5/S090WIND.U.PHYS.grb testdata/aro2.5/S090WIND.V.PHYS.grb --field[0].type VECTOR  --view-{ --lon 2 --lat 46.2 --fov 3 }- --field[0].vector.barb.color red --field[0].vector.arrow.off --coast.lines.color cyan --coast.on --field[0].vector.norm.off --field[0].vector.scale 1   --field[0].vector.barb.on
