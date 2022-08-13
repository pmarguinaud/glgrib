#!/bin/bash

set -x
set -e

export DEBEMAIL="pmarguinaud@hotmail.com"
export DEBFULLNAME="Philippe Marguinaud"


dir=glgrib_1.0-1_amd64
dird=glgrib-data_1.0-1_amd64

mkdir -p $dir $dird


function ss ()
{
  src=$1
  dst=$2
  mkdir -p $dst
  cp -alf $src/* $dst
}

if [ 0 -eq 1 ]
then

for BUILD in "" batch
do
  make BUILD=$BUILD

  rm lib/libLFI.a
  chrpath -d bin/*

  ss bin $dir/usr/bin
  ss lib $dir/lib/x86_64-linux-gnu

  make BUILD=$BUILD clean
done

for x in share/glgrib/coastlines share/glgrib/doc share/glgrib/fonts share/glgrib/landscape \
         share/glgrib/perltk share/glgrib/shaders share/glgrib/test
do
  ss $x $dir/usr/$x
done

for x in share/glgrib/glGrib.db
do
  cp -alf $x $dir/usr/$x
done

fi

ss share/glgrib/data $dird/share/glgrib/data

if [ 0 -eq 1 ]
then

cd $dir
mkdir -p debian
touch debian/control

cat -> debian/control << EOF
Package: glgrib
Version: 1.0
Architecture: amd64
Maintainer: $DEBFULLNAME <$DEBEMAIL>
Provides: glgrib 
Source: glgrib
Description: Display GRIB data with OpenGL.
 More info at https://github.com/pmarguinaud/glgrib.
EOF

dpkg-shlibdeps -O usr/bin/*  | perl -pe 's/shlibs:Depends=/Depends: /o;'  > depends.txt

cat depends.txt >> debian/control
\rm depends.txt

mv debian DEBIAN

cd ..

fi

cd $dird
mkdir -p DEBIAN
touch DEBIAN/control

cat -> DEBIAN/control << EOF
Package: glgrib-data
Version: 1.0
Architecture: amd64
Maintainer: $DEBFULLNAME <$DEBEMAIL>
Provides: glgrib 
Source: glgrib
Description: Display GRIB data with OpenGL.
 More info at https://github.com/pmarguinaud/glgrib.
EOF

cd ..

if [ 0 -eq 1 ]
then

dpkg-deb --build --root-owner-group glgrib_1.0-1_amd64 
dpkg-deb --build --root-owner-group glgrib-data_1.0-1_amd64 

fi



# dpkg -i ./glgrib_1.0-1_amd64.deb
# apt-get -f install

# apt-get remove glgrib
# apt-get autoremove
