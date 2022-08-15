#!/bin/bash

set -x
set -e 

sudo docker stop ubuntu_glgrib
sudo docker rm ubuntu_glgrib
sudo docker image rm ubuntu:latest 
