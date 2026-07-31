#!/bin/bash

exec singularity shell \
  --cwd /root/glgrib \
  --fakeroot \
  --writable \
  --bind $HOME/tmp:/tmp \
  ../singularity.glgrib-install
