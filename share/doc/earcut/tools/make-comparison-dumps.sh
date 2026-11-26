#!/bin/bash
#
# Dump the triangulations of selected polygons for the figures of the comparison report
#
#   ./tools/make-comparison-dumps.sh /path/to/workdir
#
# Run after make-comparison.sh (uses the tri_driver built there).
#

set +e

W=${1:-/tmp/tri}
TOP=$(cd $(dirname $0)/../../../.. && pwd)
TOOLS=$(cd $(dirname $0) && pwd)
# The reports were made with the GSHHS files as shipped before they were cleaned (commit 0dc6400a).
# To reproduce them exactly, extract those files and set SHP :
#   git archive 0dc6400a share/glgrib/coastlines/shp | tar -x -C DIR
#   SHP=DIR/share/glgrib/coastlines/shp
SHP=${SHP:-$TOP/share/glgrib/coastlines/shp}
NE=$TOOLS/ext/naturalearth

cd $W

dump ()
{
  local name=$1 file=$2 rank=$3
  for m in earcut earcut-fix triangle cdt
  do
    mkdir -p dumps/$name/$m
    ./tri_driver $file $m dump=dumps/$name/$m $rank > /dev/null
  done
}

# Known bad rings (earcut.pdf section 4, gshhs-data-notes.md)
dump honshu_i_L1_102218        $SHP/GSHHS_i_L1.shp 102218
dump maine_i_L1_194020         $SHP/GSHHS_i_L1.shp 194020
dump maine_h_L1_806054         $SHP/GSHHS_h_L1.shp 806054
dump neardup_i_L2_81960        $SHP/GSHHS_i_L2.shp 81960
dump obirtysh_i_L2_87722       $SHP/GSHHS_i_L2.shp 87722
dump antarctica_c_L5_0         $SHP/GSHHS_c_L5.shp 0
dump islet_h_L1_1071439        $SHP/GSHHS_h_L1.shp 1071439
dump ne_antarctica_110m_105     $NE/110m/ne_110m_land.shp 105
dump ne_pinched_lake_10m_38847  $NE/10m/ne_10m_lakes.shp 38847

# Extra cases listed on the command line as name:file:rank
shift
for c in "$@"
do
  IFS=: read name file rank <<< "$c"
  dump $name $file $rank
done
