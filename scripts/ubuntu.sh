#!/bin/bash

set -x
set -e

UBUNTU=22.04
VERSION=1.0
VERSION_P=$VERSION-1

export DEBEMAIL="pmarguinaud@hotmail.com"
export DEBFULLNAME="Philippe Marguinaud"

pwd=$PWD


dir=$pwd/$UBUNTU/glgrib_${VERSION_P}_amd64
dird=$pwd/$UBUNTU/glgrib-data_${VERSION_P}_amd64
dirp=$pwd/$UBUNTU/glgrib-perl_${VERSION_P}_amd64

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

for BUILD in batch ""
do
  make BUILD=$BUILD clean

  make BUILD=$BUILD

  rm lib/libLFI.a
  patchelf --remove-rpath bin/*

  ss bin $dir/usr/bin
  ss lib $dir/lib/x86_64-linux-gnu

done

fi

if [ 0 -eq 1 ]
then

# Libraries, executables and files

for x in share/glgrib/coastlines share/glgrib/doc share/glgrib/fonts share/glgrib/landscape \
         share/glgrib/perltk share/glgrib/shaders share/glgrib/test
do
  ss $x $dir/usr/$x
done

for x in share/glgrib/glGrib.db
do
  cp -alf $x $dir/usr/$x
done

cd $dir
mkdir -p debian
touch debian/control

cat -> debian/control << EOF
Package: glgrib
Version: $VERSION
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

cd $pwd

cd $UBUNTU
dpkg-deb --build --root-owner-group glgrib_${VERSION_P}_amd64 
cd ..

fi

if [ 0 -eq 1 ]
then

# Test data

ss share/glgrib/data $dird/share/glgrib/data

cd $dird
mkdir -p DEBIAN
touch DEBIAN/control

cat -> DEBIAN/control << EOF
Package: glgrib-data
Version: $VERSION
Architecture: amd64
Maintainer: $DEBFULLNAME <$DEBEMAIL>
Provides: glgrib 
Source: glgrib
Description: Test data for glGrib (display GRIB data with OpenGL).
 More info at https://github.com/pmarguinaud/glgrib.
Depends: glgrib (>= $VERSION)
EOF

cd $pwd

cd $UBUNTU
dpkg-deb --build --root-owner-group glgrib-data_${VERSION_P}_amd64 
cd ..

fi

if [ 1 -eq 1 ]
then

# Perl bindings

cd src/perl 

export GLGRIB_PREFIX=/usr
perl Makefile.PL
make clean
make

vvv=$(perl -e ' use English ; (my $v = $PERL_VERSION) =~ s/^v//o; print $v ')
vv=$(perl -e ' use English ; (my $v = $PERL_VERSION) =~ s/^v//o; $v =~ s/\.\d+$//o; print $v ')

mkdir -p $dirp/usr/lib/x86_64-linux-gnu/perl/$vv
mkdir -p $dirp/usr/lib/x86_64-linux-gnu/perl/$vvv/auto/glGrib
mkdir -p $dirp/usr/share/man/man3

\rm -f $dirp/usr/lib/x86_64-linux-gnu/perl/$vv/glGrib.pm
cp blib/lib/glGrib.pm $dirp/usr/lib/x86_64-linux-gnu/perl/$vv/glGrib.pm

\rm -f $dirp/usr/lib/x86_64-linux-gnu/perl/$vvv/auto/glGrib/glGrib.so

cp blib/arch/auto/glGrib/glGrib.so $dirp/usr/lib/x86_64-linux-gnu/perl/$vvv/auto/glGrib/glGrib.so
patchelf --set-rpath /lib/x86_64-linux-gnu $dirp/usr/lib/x86_64-linux-gnu/perl/$vvv/auto/glGrib/glGrib.so

cp blib/man3/glGrib.3pm $dirp/usr/share/man/man3/glGrib.3pm
\rm -f $dirp/usr/share/man/man3/glGrib.3pm.gz
gzip $dirp/usr/share/man/man3/glGrib.3pm

cd ../..


cd $dirp

mkdir -p DEBIAN
touch DEBIAN/control

cat -> DEBIAN/control << EOF
Package: glgrib-perl
Version: $VERSION
Architecture: amd64
Maintainer: $DEBFULLNAME <$DEBEMAIL>
Provides: glgrib 
Source: glgrib
Description: Perl bindings for glGrib (display GRIB data with OpenGL).
 More info at https://github.com/pmarguinaud/glgrib.
Depends: glgrib (>= $VERSION), perl (>= 5.16.0)
EOF

cd $pwd

cd $UBUNTU
dpkg-deb --build --root-owner-group glgrib-perl_${VERSION_P}_amd64 
cd ..

fi


# dpkg -i ./glgrib_${VERSION_P}_amd64.deb
# apt-get -f install

# apt-get remove glgrib
# apt-get autoremove
