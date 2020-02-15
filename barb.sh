#!/bin/bash

set -x

make

./glgrib.x --field[0].path testdata/aro2.5/S090WIND.U.PHYS.grb testdata/aro2.5/S090WIND.V.PHYS.grb --field[0].type VECTOR  --view-{ --lon 2 --lat 46.2 --fov 3 }- --field[0].vector.arrow.color red --coast.lines.color cyan --coast.off --field[0].vector.norm.off --field[0].vector.arrow.kind 3 --field[0].vector.arrow.fill.off --field[0].vector.scale 5  --field[0].vector.arrow.fixed.on   --view.fov 0.1 --view.lat 41.4841 --view.lon 6.03527
