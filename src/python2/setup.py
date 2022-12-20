from setuptools import setup, Extension
import os
import sys
import platform
import re

TOP = "../.."
prefix = TOP


debian = os.getenv ('DEB_BUILD_ARCH')

if (sys.argv[1] == 'install') and (debian is not None):
  version = re.sub ("\.\d+$", "", platform.python_version ())
  dir = TOP + '/debian/tmp/usr/lib/python' + version + '/site-packages/'
  os.environ['PYTHONPATH'] = dir
  if not  os.path.exists (dir):
    os.makedirs (dir)
  sys.argv.append ('--prefix=' + prefix + "/debian/tmp/usr")

setup (
    name = "glGrib",
    version = "0.1",
    ext_modules = [Extension ("glGrib", ["glGrib.cc"], 
    include_dirs=[TOP + '/include'], 
    define_macros=[('GLGRIB_PREFIX', '"' + prefix + '"'),('USE_GLFW','1')],
    libraries=['glGrib', 'LFI', 'GLEW', 'GL', 'glfw', 'png', 'readline', 'ncurses',
               'tinfo', 'ssl', 'crypto', 'pthread', 'sqlite3', 'curl', 'shp', 'eccodes'],
    library_dirs=[prefix + "/lib", "../lib"])],
  );

