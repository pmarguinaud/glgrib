#!/usr/bin/python3

import glGrib
import time
import os
import glob

def main():
  glGrib.start ('--render.width', 1200);
  print glGrib.json ();
  glGrib.set ('--field[0].path', '../../share/glgrib/testdata/t1798/Z.grb')
  glGrib.set ('--colorbar.on');
  glGrib.sleep (10);
  glGrib.stop ();
  

if __name__ == "__main__":
    main()
