#!/bin/bash
#
# Run the comparison of triangulation methods (see ../triangulation-comparison-plan.md)
#
#   ./tools/make-comparison.sh /path/to/workdir
#
# Needs tools/ext/{triangle,cdt,naturalearth} (see the report for the download commands),
# and the Natural Earth 3.0.1 files shipped with Magics in $MAGICS_NE (optional).
#

set -e

W=${1:-/tmp/tri}
TOP=$(cd $(dirname $0)/../../../.. && pwd)
TOOLS=$(cd $(dirname $0) && pwd)
SHP=$TOP/share/glgrib/coastlines/shp
NE=$TOOLS/ext/naturalearth
MAGICS_NE=${MAGICS_NE:-}

$TOOLS/build-tri-driver.sh $W
cd $W

# Do not stop on a crash of one method : record it in the output file
set +e
run ()
{
  "$@"
  local rc=$?
  [ $rc != 0 ] && echo "# CRASH exit $rc"
  return 0
}

METHODS="earcut earcut-fix triangle triangle-Y cdt cdt-noX cdt-outer"

# Whole layers, serial, with checks
mkdir -p layers
for f in c_L1 c_L2 c_L3 c_L5 c_L6 i_L1 i_L2 i_L3 i_L4 i_L5 i_L6 h_L1 h_L2 h_L3 h_L4 h_L5 h_L6
do
  for m in $METHODS
  do
    run ./tri_driver $SHP/GSHHS_$f.shp $m all > layers/gshhs_${f}_$m.txt
  done
done

for s in 110m 50m 10m
do
  for k in land lakes
  do
    for m in $METHODS
    do
      run ./tri_driver $NE/$s/ne_${s}_$k.shp $m all > layers/ne511_${s}_${k}_$m.txt
    done
  done
  if [ -n "$MAGICS_NE" ]
  then
    for m in $METHODS
    do
      run ./tri_driver $MAGICS_NE/$s/ne_${s}_land.shp $m all > layers/ne301_${s}_land_$m.txt
    done
  fi
done

# Timings : whole layers without checks, serial and parallel, 3 runs each (keep the best)
mkdir -p timings
for f in GSHHS_c_L1 GSHHS_i_L1 GSHHS_h_L1 GSHHS_h_L2
do
  for m in earcut earcut-fix triangle cdt
  do
    for mode in serial par
    do
      p=""; [ $mode = par ] && p=par
      for r in 1 2 3
      do
        ./tri_driver $SHP/$f.shp $m nocheck $p all | tail -1
      done > timings/${f}_${m}_$mode.txt
    done
  done
done
for s in 110m 50m 10m
do
  for m in earcut earcut-fix triangle cdt
  do
    for mode in serial par
    do
      p=""; [ $mode = par ] && p=par
      for r in 1 2 3
      do
        ./tri_driver $NE/$s/ne_${s}_land.shp $m nocheck $p all | tail -1
      done > timings/ne_${s}_land_${m}_$mode.txt
    done
  done
done

# Memory (peak RSS) for the largest layer
mkdir -p memory
for m in earcut earcut-fix triangle cdt
do
  /usr/bin/time -v ./tri_driver $SHP/GSHHS_h_L1.shp $m nocheck all 2> memory/h_L1_$m.txt > /dev/null
done
