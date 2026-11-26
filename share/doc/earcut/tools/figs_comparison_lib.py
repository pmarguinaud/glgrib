#!/usr/bin/env python3
# Readers shared by figs_comparison.py and quality_comparison.py

import numpy as np


def readDump(f):
    V, T, B = [], [], []
    st = ''
    for l in open(f):
        t = l.split()
        if t[0] == 'V':
            V.append((float(t[1]), float(t[2])))
        elif t[0] == 'T':
            T.append((int(t[1]), int(t[2]), int(t[3])))
        elif t[0] == 'B':
            B.append((int(t[1]), int(t[2])))
        elif t[0] == 'R':
            st = t[4]
    return np.array(V), np.array(T, dtype=int).reshape(-1, 3), B, st


def xyz(lonlat):
    lon, lat = np.radians(lonlat[:, 0]), np.radians(lonlat[:, 1])
    return np.stack([np.cos(lon) * np.cos(lat), np.sin(lon) * np.cos(lat), np.sin(lat)], axis=1)


