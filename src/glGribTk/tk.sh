#!/bin/bash


export GLGRIB_PREFIX=../..

perl -Mblib=../perl -Mblib bin/glGribTk.pl $*
