#!/usr/bin/python3

import glGrib
import time
import os
import glob

def main():
  glGrib.start ('--grid.on', '--landscape.on');
  print glGrib.get ('--grid');
  print glGrib.window ();
  glGrib.sleep (2);
  glGrib.set ('--grid.off');
  glGrib.sleep (2);
  glGrib.stop ();
  

if __name__ == "__main__":
    main()
