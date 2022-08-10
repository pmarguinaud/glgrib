#!/bin/bash

set -x
set -e

dir=glgrib_1.0-1_amd64
mkdir -p $dir


function ss ()
{
  src=$1
  dst=$2
  mkdir -p $dir/$dst
  cp -alf $src/* $dir/$dst
}

for BUILD in "" batch
do
  make BUILD=$BUILD

  rm lib/libLFI.a
  chrpath -d bin/*

  ss bin usr/bin
  ss lib lib/x86_64-linux-gnu
  ss share/glgrib usr/share/glgrib

  make BUILD=$BUILD clean
done


export DEBEMAIL="pmarguinaud@hotmail.com"
export DEBFULLNAME="Philippe Marguinaud"

cd $dir
mkdir -p debian
touch debian/control

cat -> debian/control << EOF
Package: glgrib
Version: 1.0
Architecture: amd64
Maintainer: $DEBFULLNAME <$DEBEMAIL>
Provides: glgrib glgrib-batch
Source: glgrib
Description: Display GRIB data with OpenGL.
 More info at https://github.com/pmarguinaud/glgrib.
EOF

dpkg-shlibdeps -O usr/bin/*  | perl -pe 's/shlibs:Depends=/Depends: /o;'  > depends.txt

cat depends.txt >> debian/control
\rm depends.txt

mv debian DEBIAN

cd ..

exit
dpkg-deb --build --root-owner-group glgrib_1.0-1_amd64 



# dpkg -i ./glgrib_1.0-1_amd64.deb
# apt-get -f install

# apt-get remove glgrib
# apt-get autoremove
