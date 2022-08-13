#!/bin/bash

set -x
set -e 

sudo docker stop ubuntu 
sudo docker rm ubuntu 
sudo docker image rm ubuntu:latest 
