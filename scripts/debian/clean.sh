#!/bin/bash

set -e

if [ ! -d debian ]
then
  exit
fi

cd debian

set -x
\rm -rf tmp files debhelper-build-stamp .debhelper
set +x

for f in *.install
do
  b=$(basename $f .install)
  set -x
  \rm -rf $b $b.substvars
  set +x
done

