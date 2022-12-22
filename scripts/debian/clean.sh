#!/bin/bash

set -x
set -e

cd debian

\rm -rf tmp files debhelper-build-stamp .debhelper

for f in *.install
do
  b=$(basename $f .install)
  \rm -rf $b $b.substvars
done
