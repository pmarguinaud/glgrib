#!/bin/bash


export GLGRIB_PREFIX=../..

perl -Mblib=../perl/glfw -Mblib bin/glgrib-tk $*
