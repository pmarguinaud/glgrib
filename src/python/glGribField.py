#!/usr/bin/python

import glGrib
import time
import os
import glob

def main():
  glGrib.start ();
  print glGrib.json ();
  glGrib.set ('--colorbar.on', '--window.width', '1200')
  glGrib.set ('--field[0].path', '/home/phi001/3d/glgrib/XYZ1.pack.fa%SURFX')
  glGrib.sleep (10);
  glGrib.stop ();
  

if __name__ == "__main__":
    main()
