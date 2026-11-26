#!/bin/bash
#
# Clean all GSHHS shapefiles shipped with glgrib
#
#   src/gshhsClean/clean-gshhs.sh OUTDIR
#
# Writes OUTDIR/GSHHS_<res>_L<level>.{shp,shx,dbf,prj} and OUTDIR/GSHHS_<res>_L<level>.log,
# and prints the summary line of each file. The files in share/glgrib/coastlines/shp are
# not modified ; copy the cleaned files there by hand once the logs have been reviewed.
#

set -e

OUT=${1:?Usage: $0 OUTDIR}
DIR=$(cd $(dirname $0) && pwd)
TOP=$(cd $DIR/../.. && pwd)
SHP=$TOP/share/glgrib/coastlines/shp

make -s -C $DIR gshhs_clean

mkdir -p $OUT
OUT=$(cd $OUT && pwd)

for f in $SHP/GSHHS_*.shp
do
  b=$(basename $f .shp)
  $DIR/gshhs_clean $f $OUT/$b.shp > $OUT/$b.log
  tail -1 $OUT/$b.log | sed -e "s,^# file .*/,,"
done
