#!/bin/bash
#
# Build the Debian packages (run from the top of the source tree, in the
# build container). The packages are built from a copy of the files tracked
# by git (the .orig.tar.gz made by tar.pl), so that build products and local
# files of the working tree do not end up in the source package. The results
# are moved to .. (where debian-install.sh expects them).

set -e

top=$(pwd)
dir=$(basename $top)
work=$top/../debian-build

./scripts/debian/tar.pl > /dev/null

rm -rf $work
mkdir $work
mv ../${dir}_*.orig.tar.gz $work/

cd $work
tar xf ${dir}_*.orig.tar.gz
cp -r $top/packaging/debian $dir/debian

cd $dir
debuild -us -uc

cd $work
mv *.deb *.dsc *.changes *.buildinfo *.build *.debian.tar.* *.orig.tar.gz ..
cd ..
rm -rf $work
