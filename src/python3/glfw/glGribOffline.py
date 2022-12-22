#!/usr/bin/python3

import glGrib.glfw
import time
import os
import glob

def main():
  for f in glob.glob ("snapshot*.png"):
     os.remove (f)
  glGrib.glfw.start ('--grid.on', '--landscape.on', '--render.offscreen.on');
  glGrib.glfw.snapshot ();
  glGrib.glfw.set ('--grid.off')
  glGrib.glfw.snapshot ();
  glGrib.glfw.stop ();
  

if __name__ == "__main__":
    main()
