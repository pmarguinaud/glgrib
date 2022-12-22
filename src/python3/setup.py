from setuptools import setup, Extension
import os
import sys

TOP = "../.."
prefix = TOP

debian = os.getenv ('DEB_BUILD_ARCH')

if (sys.argv[1] == 'install') and (debian is not None):
  sys.argv.append ('--prefix=' + prefix + "/debian/tmp/usr")
  sys.argv.append ('--old-and-unmanageable')

if (debian is not None):
  prefix = '/usr'

setup (
    name = "glGrib.glfw",
    version = "0.1",
    ext_modules = [Extension ("glGrib.glfw", ["glfw.cc"], 
    include_dirs=[TOP + '/include'], 
    define_macros=[('GLGRIB_PREFIX', '"' + prefix + '"'),('USE_GLFW','1')],
    libraries=['glGrib-glfw', 'LFI', 'GLEW', 'GL', 'glfw', 'png', 'readline', 'ncurses',
               'tinfo', 'ssl', 'crypto', 'pthread', 'sqlite3', 'curl', 'shp', 'eccodes'],
    library_dirs=[prefix + "/lib", TOP + "/lib"])],
  );

