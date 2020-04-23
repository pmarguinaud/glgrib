#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import print_function, absolute_import, unicode_literals, division
import six

import argparse
import os 
import glGrib


def is_valid_file(parser, arg):
    if not os.path.exists(arg):
        parser.error("The file %s does not exist!" % arg)
    else:
        #ajouter un test sur le format de fichier
        if True:
            pass
        else:
            parser.error("The file %s is not a grib nor an FA" % arg)
        return arg  

def main(file_path=None,cmd=None,info=None,all=None,field=None):

    if cmd:
        try:
            command=([k.encode('utf-8') for k in cmd.split(" ")])
        except:
            raise argparse.ArgumentTypeError('--cmd argument not valid: {}'.format(cmd))
        glGrib.start(*command)
        glGrib.sleep(200)
    elif info:
        if field:
            print("TODO : print info on field {} in file {} ".format(field,info))
        else:
            print("TODO : print info on file {} ".format(info))
    elif file_path:
        if field:
            print("TODO : plot field {} of file {}".format(field,file_path))
        elif all:
            print("TODO : plot all fields of file ",file_path)
    else:
        print("DBG :",file_path)

helpCmd="""Mimic the raw glgrib binary command line behaviour.\n `gl_plot.py --cmd "glgrib_options"`
is equivalent to \n
`glbrib glgrib_options` (the `"` are needed around glgrib_options)"""

helpI="""Display informations on the file `file_path` 
"""

helpA="""Plot all fields of `file_path` 
"""

helpF="""Plot only the field `field_name`"""

helpPath="""This is the main argument of the program. Must be None with the -c/--cmd option"""

if __name__ == "__main__":
    # 1. Parse arguments
    ####################
    parser = argparse.ArgumentParser(description="This tool is a python interface to the glGrib 3d visualisation program",
                                     epilog='Thanks for using glGrib')
    #soit on donne une commande brute, soit on doit specifier un fichier, soit on print les infos
    group = parser.add_mutually_exclusive_group()
    group.add_argument('--cmd','-c',metavar='glgrib_options',help=helpCmd,default=None)
    group.add_argument('--file_path','-p',help=helpPath,
                        type=lambda x: is_valid_file(parser, x),default=None)   
    group.add_argument('--info','-i', help=helpI,
                       type=lambda x: is_valid_file(parser, x),default=None)
    #soit on specifie un champ, sinon on les trace tous
    groupField = parser.add_mutually_exclusive_group()
    groupField.add_argument('--all','-a',help=helpA,action='store_true',
                       default=True)
    groupField.add_argument('--field','-f',metavar='field_name', help=helpF,default=None)    
    

    
    args = parser.parse_args()

    
    main(**vars(args))
    
    
    
    
