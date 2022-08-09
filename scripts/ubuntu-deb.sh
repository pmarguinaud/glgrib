#!/bin/bash

set -x

dir=glgrib_1.0_1
mkdir -p $dir


function ss ()
{
  src=$1
  dst=$2
  mkdir -p $dir/$dst
  cp -alf $src $dir/$dst
}


ss bin usr/bin
ss lib lib/x86_64-linux-gnu
ss share/glgrib usr/share/glgrib



