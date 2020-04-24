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


    
def main(file_path=None,info=None,field=None,diff=None):

    if info:
        if field:
            print("TODO : print info on field {} in file {} ".format(field,info))
        else:
            print("TODO : print info on file {} ".format(info))
    else: 
        if not diff:
            listCmd=[]
            listCmd.append('--window.fullscreen.on')
            listCmd.append('--coast.on')
            listCmd.append('--colorbar.on')
            listCmd.append('--scene.title.on')
            listCmd.append('--view.projection')
            listCmd.append('latlon')
            listCmd.append('--review.on')
            listCmd.append('--review.path')
            listCmd.append(file_path)
            listCmd.append('--field[0].palette.name')
            listCmd.append('cold_hot')
            command=(listCmd)  
            glGrib.start(*command)
            glGrib.sleep(200)
        else:
            listCmd=[]
            listCmd.append('--coast.on')
            listCmd.append('--colorbar.on')
            listCmd.append('--scene.title.on')
            listCmd.append('--view.projection')
            listCmd.append('latlon')
            listCmd.append('--diff.on')
            listCmd.append('--diff.path')
            listCmd.append(file_path)
            listCmd.append(diff)
            listCmd.append('--field[0].palette.name')
            listCmd.append('cold_hot')
            command=(listCmd)  
            print(*command)
            glGrib.start(*command)
            glGrib.sleep(200)



helpCmd="""Mimic the epy_plot.py command line behaviour, but using glgrib.\n
"""
helpI="""Display informations on the file `file_path` 
"""

helpDiff="""Plot differences between `file_path` and `diff`
"""

helpF="""Plot only the field `field_name`, else plot all fields (navigation with page up page down)"""

helpPath="""Path of the file to plot"""


if __name__ == "__main__":
    # 1. Parse arguments
    ####################
    parser = argparse.ArgumentParser(description="This tool is a python interface to the glGrib 3d visualisation program",
                                     epilog='Thanks for using glGrib')
    parser.add_argument('file_path',help=helpPath,
                        type=lambda x: is_valid_file(parser, x),default=None)   
    parser.add_argument('--info','-i', help=helpI,
                       type=lambda x: is_valid_file(parser, x),default=None)
    parser.add_argument('--field','-f',metavar='field_name', help=helpF,default=None)    
    
    parser.add_argument('--diff','-d',metavar='path_file_2', help=helpDiff,default=None)    
    
    args = parser.parse_args()

    main(**vars(args))
    
    
    
    
