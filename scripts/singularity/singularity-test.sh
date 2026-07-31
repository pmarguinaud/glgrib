#!/bin/bash

exec singularity shell --bind $HOME/tmp:/tmp ../singularity.glgrib-install
