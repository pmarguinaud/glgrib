#!/bin/bash
#
# Reproduce the data and the figures of earcut.tex
#
# EarCut is the copy in tools/earcut : glgrib no longer contains it (removed after CDT was adopted).
#
# 1. apply the instrumentation patch to tools/earcut/EarCut.cc (from the top of the repository) :
#      git apply share/doc/earcut/tools/earcut-instrumentation.patch
# 2. run this script from share/doc/earcut :
#      ./tools/make-figures.sh /path/to/workdir
# 3. restore EarCut.cc :
#      git checkout share/doc/earcut/tools/earcut/EarCut.cc
#
# Environment variables read by the instrumented EarCut.cc :
#   GLGRIB_EARCUT_DUMP=dir      write dir/stats.txt (one line per ring) and dir/ring_<rank>.txt
#   GLGRIB_EARCUT_DUMP_MIN=N    write ring_<rank>.txt for rings with N vertices or more
#   GLGRIB_EARCUT_NOOMP=1       force openmp = false in processRing
#   GLGRIB_EARCUT_FIXROT=1      use the transposed rotation matrix (xyz2 = R^T xyz1)
#   GLGRIB_EARCUT_DEBUG=j       trace the ear test of vertex j (local index in the ring)

set -e

W=${1:-/tmp/earcut}
TOP=$(cd $(dirname $0)/../../../.. && pwd)
DOC=$(cd $(dirname $0)/.. && pwd)
# The reports were made with the GSHHS files as shipped before they were cleaned (commit 0dc6400a).
# To reproduce them exactly, extract those files and set SHP :
#   git archive 0dc6400a share/glgrib/coastlines/shp | tar -x -C DIR
#   SHP=DIR/share/glgrib/coastlines/shp
SHP=${SHP:-$TOP/share/glgrib/coastlines/shp}

mkdir -p $W/data
cd $W

g++ -O2 -fopenmp -std=c++11 -I$DOC/tools/earcut -I$TOP/include $DOC/tools/earcut_driver.cc $DOC/tools/earcut/EarCut.cc -lshp -o earcut_driver

# Statistics over whole layers
for fix in 0 1
do
  for m in omp noomp
  do
    if [ $fix = 1 ]; then export GLGRIB_EARCUT_FIXROT=1; else unset GLGRIB_EARCUT_FIXROT; fi
    for r in c i h
    do
      d=data/st_${r}_${m}_fix$fix; mkdir -p $d
      GLGRIB_EARCUT_DUMP=$d ./earcut_driver $SHP/GSHHS_${r}_L1.shp $m all > $d/times.txt
    done
    d=data/st_hL2_${m}_fix$fix; mkdir -p $d
    GLGRIB_EARCUT_DUMP=$d ./earcut_driver $SHP/GSHHS_h_L2.shp $m all > $d/times.txt
  done
done

# Individual rings
for fix in 0 1
do
  for m in omp noomp
  do
    if [ $fix = 1 ]; then export GLGRIB_EARCUT_FIXROT=1; else unset GLGRIB_EARCUT_FIXROT; fi
    d=data/rd_c_${m}_fix$fix;   mkdir -p $d
    GLGRIB_EARCUT_DUMP=$d GLGRIB_EARCUT_DUMP_MIN=3 ./earcut_driver $SHP/GSHHS_c_L1.shp $m 0 1031 2230 1252 > /dev/null
    d=data/rd_i_${m}_fix$fix;   mkdir -p $d
    GLGRIB_EARCUT_DUMP=$d GLGRIB_EARCUT_DUMP_MIN=3 ./earcut_driver $SHP/GSHHS_i_L1.shp $m 102218 194020 > /dev/null
    d=data/rd_iL2_${m}_fix$fix; mkdir -p $d
    GLGRIB_EARCUT_DUMP=$d GLGRIB_EARCUT_DUMP_MIN=3 ./earcut_driver $SHP/GSHHS_i_L2.shp $m 87722 > /dev/null
  done
done

unset GLGRIB_EARCUT_FIXROT
PYTHONPATH=$DOC/tools python3 $DOC/tools/figs.py data $DOC/fig
