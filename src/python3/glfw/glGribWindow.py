#!/usr/bin/python3

import glGrib.glfw
import time
import os
import glob

def main():
  glGrib.glfw.start ('--grid.on', '--landscape.on');
  print (glGrib.glfw.get ('--grid'));
  print (glGrib.glfw.window ());
  glGrib.glfw.sleep (2);
  glGrib.glfw.set ('--grid.off');
  glGrib.glfw.sleep (2);
  glGrib.glfw.stop ();
  

if __name__ == "__main__":
    main()
