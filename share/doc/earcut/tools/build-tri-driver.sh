#!/bin/bash
#
# Build tri_driver in directory $1 (default /tmp/tri)
# Needs tools/ext/triangle (Triangle 1.6, from netlib) and tools/ext/cdt/src (CDT 2.1.0).
# EarCut is the copy in tools/earcut : glgrib no longer contains it (removed after CDT was adopted).
#

set -e

W=${1:-/tmp/tri}
TOP=$(cd $(dirname $0)/../../../.. && pwd)
TOOLS=$(cd $(dirname $0) && pwd)
EXT=$TOOLS/ext

mkdir -p $W/build
cd $W/build

# EarCut as it was in glgrib
g++ -O2 -fopenmp -std=c++17 -I$TOOLS/earcut -I$TOP/include -c $TOOLS/earcut/EarCut.cc -o EarCut.o

# EarCut fixed : transposed rotation and serial passes
sed -e 's/xyz2\[i\] = R \* xyz1\[i\];/xyz2[i] = glm::transpose (R) * xyz1[i];/' \
    -e '/int rank1 = rankb, rank2 = rankb + rankl;/i\  openmp = false; // EarCut fixed : serial passes' \
    $TOOLS/earcut/EarCut.cc > EarCutFix.cc
grep -q 'glm::transpose (R) \* xyz1' EarCutFix.cc
grep -q 'openmp = false; // EarCut fixed' EarCutFix.cc
g++ -O2 -fopenmp -std=c++17 -DglGrib=glGribFix -I$TOOLS/earcut -I$TOP/include -c EarCutFix.cc -o EarCutFix.o
g++ -O2 -fopenmp -std=c++17 -DglGrib=glGribFix -I$TOOLS/earcut -I$TOP/include -c $TOOLS/earcut_fix_wrap.cc -o earcut_fix_wrap.o

# Triangle : triexit () calls exit () ; redirect it to a hook that longjmp's back to the driver
sed -e 's/^  exit(status);$/  triangle_exit_hook(status);/' \
    -e 's/^void triexit(int status)$/extern void triangle_exit_hook(int);\nvoid triexit(int status)/' \
    $EXT/triangle/triangle.c > triangle_lj.c
grep -q 'triangle_exit_hook(status)' triangle_lj.c
gcc -O2 -I$EXT/triangle -DTRILIBRARY -DANSI_DECLARATORS -DREAL=double -DVOID=void -DNO_TIMER -c triangle_lj.c -o triangle.o -w

# Driver (CDT is header-only)
g++ -O2 -fopenmp -std=c++17 -I$TOOLS/earcut -I$TOP/include -I$EXT/triangle -I$EXT/cdt/src/CDT/include \
    -c $TOOLS/tri_driver.cc -o tri_driver.o
g++ -fopenmp -o $W/tri_driver tri_driver.o EarCut.o EarCutFix.o earcut_fix_wrap.o triangle.o -lshp

echo "Built $W/tri_driver"
