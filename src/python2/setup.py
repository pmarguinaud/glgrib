from setuptools import setup, Extension
import unittest
import os

def my_test_suite ():
    test_loader = unittest.TestLoader ()
    test_suite = test_loader.discover ('tests', pattern='*.py')
    return test_suite


TOP = "../.."
prefix = TOP

setup (
    name = "glGrib",
    version = "0.1",
    test_suite="setup.my_test_suite",

    ext_modules = [Extension ("glGrib", ["glGrib.cc"], 
    include_dirs=[TOP + '/include'], 
    define_macros=[('GLGRIB_PREFIX', '"' + prefix + '"')],
    libraries=['glGrib', 'LFI', 'GLEW', 'GL', 'glfw', 'png', 'readline', 'ncurses',
               'tinfo', 'ssl', 'crypto', 'pthread', 'sqlite3', 'curl', 'shp', 'eccodes'],
    runtime_library_dirs=[prefix + "/lib", "../lib"],
    library_dirs=[prefix + "/lib", "../lib"])],

  );

