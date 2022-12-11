#!/bin/bash

set -x
set -e 

for kind in build install
do
  sudo docker stop debian_glgrib_${kind}
  sudo docker rm debian_glgrib_${kind}
done


