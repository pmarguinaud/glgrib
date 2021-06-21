#!/bin/bash

ffmpeg  -i TEST_%04d.png -c:v libx264 -vf fps=25 -pix_fmt yuv420p TEST.mp4
