#!/bin/bash

set -x
set -e 

for kind in build install
do
  sudo docker stop glgrib_${kind}
  sudo docker rm glgrib_${kind}
  \rm -f grib_${kind}.txt
done


