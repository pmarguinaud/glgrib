#!/usr/bin/python

import glGrib
import time

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
