#!/usr/bin/env python3
#
# Triangle quality (smallest angle) for the dumped polygons of the comparison
#
#   python3 quality_comparison.py <work dir>

import sys, os, glob
import numpy as np
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from figs_comparison_lib import readDump, xyz

W = sys.argv[1]
print('%-28s %-11s %6s %10s %10s %10s' % ('case', 'method', 'ntri', 'median', '<5 deg', '<1 deg'))
for case in sorted(os.listdir(os.path.join(W, 'dumps'))):
    for m in ['earcut', 'earcut-fix', 'triangle', 'cdt']:
        f = glob.glob(os.path.join(W, 'dumps', case, m, 'poly_*.txt'))
        if not f:
            continue
        V, T, B, st = readDump(f[0])
        if len(T) == 0:
            continue
        P = xyz(V)
        A, Bv, C = P[T[:, 0]], P[T[:, 1]], P[T[:, 2]]
        def ang(p, q, r):
            # angle at p between great circles p-q and p-r (tangent vectors)
            u = q - p * np.einsum('ij,ij->i', p, q)[:, None]
            v = r - p * np.einsum('ij,ij->i', p, r)[:, None]
            nu, nv = np.linalg.norm(u, axis=1), np.linalg.norm(v, axis=1)
            c = np.einsum('ij,ij->i', u, v) / np.maximum(nu * nv, 1e-300)
            return np.degrees(np.arccos(np.clip(c, -1, 1)))
        a = np.minimum(np.minimum(ang(A, Bv, C), ang(Bv, C, A)), ang(C, A, Bv))
        print('%-28s %-11s %6d %10.2f %9.1f%% %9.1f%%' % (case, m, len(T), np.median(a),
              100 * np.mean(a < 5), 100 * np.mean(a < 1)))
