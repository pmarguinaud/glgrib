"""Load ring dumps written by the EARCUT_DUMP instrumentation of EarCut.cc."""

import numpy as np


class Ring:
    def __init__(self, path):
        P, I, T = [], [], []
        with open(path) as fh:
            for line in fh:
                w = line.split()
                if w[0] == 'N':
                    self.n, self.omp = int(w[1]), int(w[2])
                elif w[0] == 'P':
                    P.append([float(x) for x in w[2:]])
                elif w[0] == 'I':
                    I.append([int(x) for x in w[1:]])
                elif w[0] == 'T':
                    T.append([int(x) for x in w[1:]])
        P = np.array(P)
        self.lon, self.lat = np.degrees(P[:, 0]), np.degrees(P[:, 1])
        self.rlon, self.rlat = np.degrees(P[:, 2]), np.degrees(P[:, 3])
        self.ang0 = P[:, 4]
        self.passes = np.array(I).reshape(-1, 3)        # pass, remaining, reflex
        self.tri = np.array(T).reshape(-1, 4)            # pass, apex, prev, next

    def alive(self, k):
        """Mask of vertices still in the ring after pass k (k = -1 : initial)."""
        m = np.ones(self.n, bool)
        m[self.tri[self.tri[:, 0] <= k, 1]] = False
        return m

    def xy(self, rotated=False, unwrap=True):
        x = self.rlon if rotated else self.lon
        y = self.rlat if rotated else self.lat
        if unwrap:
            x = np.degrees(np.unwrap(np.radians(x)))
        return x, y


def segments_cross(x, y):
    """List of pairs (i, j) of non-adjacent ring edges that intersect."""
    n = len(x)
    A = np.stack([x, y], 1)
    B = np.roll(A, -1, 0)
    out = []
    for i in range(n):
        p, r = A[i], B[i] - A[i]
        q, s = A, B - A
        rxs = r[0] * s[:, 1] - r[1] * s[:, 0]
        qp = q - p
        with np.errstate(divide='ignore', invalid='ignore'):
            t = (qp[:, 0] * s[:, 1] - qp[:, 1] * s[:, 0]) / rxs
            u = (qp[:, 0] * r[1] - qp[:, 1] * r[0]) / rxs
        hit = (rxs != 0) & (t > 0) & (t < 1) & (u > 0) & (u < 1)
        for j in np.nonzero(hit)[0]:
            if j > i and j not in ((i + 1) % n, (i - 1) % n):
                out.append((i, j))
    return out
