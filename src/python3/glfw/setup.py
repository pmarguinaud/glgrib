from setuptools import setup, Extension
import os
import sys

TOP = "../../.."
prefix = TOP

debian = os.getenv ('DEB_BUILD_ARCH')

if (debian is not None):
  prefix = '/usr'

setup (
    name = "glGrib.glfw",
    version = "0.1",
    ext_modules = [Extension ("glGrib.glfw", ["glfw.cc"], 
    include_dirs=[TOP + '/include'], 
    define_macros=[('GLGRIB_PREFIX', '"' + prefix + '"'),('USE_GLFW','1')],
    libraries=['glGrib-glfw'],
    library_dirs=[prefix + "/lib", TOP + "/lib"])],
  );

