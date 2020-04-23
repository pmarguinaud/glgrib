#!/usr/bin/python

import glGrib
import time
import os
import glob

def main():
  glGrib.start ('--colorbar.on', '--window.width', '1200')
  glGrib.set ('--field[0].path', '/home/phi001/3d/glgrib/XYZ1.pack.fa%SURFX')
  glGrib.sleep (2);
  glGrib.stop ();
  

if __name__ == "__main__":
    main()
