#!/usr/bin/python3

import glGrib.glfw
import time
import os
import glob

def main():
  glGrib.glfw.start ('--render.width', 1200);
  print (glGrib.glfw.json ())
  glGrib.glfw.set ('--field[0].path', '../../share/glgrib/testdata/t1798/Z.grb')
  glGrib.glfw.set ('--colorbar.on');
  glGrib.glfw.sleep (10);
  glGrib.glfw.stop ();
  

if __name__ == "__main__":
    main()
