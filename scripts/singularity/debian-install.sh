#!/bin/bash

chmod 1777 /tmp
dpkg-scanpackages -m . > Packages
xz Packages
apt-get update -y
apt install -y glgrib-glfw
apt-cache policy glgrib-glfw

