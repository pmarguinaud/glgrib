#!/usr/bin/python3

import glGrib
import time
import os
import glob

def main():
  for f in glob.glob ("snapshot*.png"):
     os.remove (f)
  glGrib.start ('--grid.on', '--landscape.on', '--render.offscreen.on');
  glGrib.snapshot ();
  glGrib.set ('--grid.off')
  glGrib.snapshot ();
  glGrib.stop ();
  

if __name__ == "__main__":
    main()
