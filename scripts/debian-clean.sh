#!/bin/bash

set -x
set -e 

for kind in build install
do
  sudo docker stop glgrib_${kind}
  sudo docker rm grib_${kind}
done


