#!/usr/bin/env python3
#
# Figures for the comparison of triangulation methods
#
#   python3 figs_comparison.py <work dir> <output dir>
#
# <work dir> is the directory filled by make-comparison.sh (layers/, timings/, dumps/)

import sys, os, glob, re
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.collections import PolyCollection, LineCollection
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from figs_comparison_lib import readDump, xyz

W, OUT = sys.argv[1], sys.argv[2]
os.makedirs(OUT, exist_ok=True)

COLORS = {'earcut': '#c0392b', 'earcut-fix': '#e67e22', 'triangle': '#2471a3',
          'triangle-Y': '#5dade2', 'cdt': '#1e8449', 'cdt-noX': '#58d68d', 'cdt-outer': '#7d3c98'}
LABELS = {'earcut': 'EarCut (glgrib)', 'earcut-fix': 'EarCut fixed', 'triangle': 'Triangle',
          'triangle-Y': 'Triangle -Y', 'cdt': 'CDT', 'cdt-noX': 'CDT no-X', 'cdt-outer': 'CDT outer'}


def readLayer(f):
    rows = []
    for l in open(f):
        if l.startswith('#') or not l[:1].isdigit():
            continue
        t = l.split()
        rows.append(dict(rank=int(t[0]), N=int(t[1]), nholes=int(t[2]), ntri=int(t[3]),
                         expected=int(t[4]), steiner=int(t[5]), status=t[6], area_err=float(t[7]),
                         overlap=float(t[8]), inverted=int(t[9]), missing=int(t[10]),
                         t_proj=float(t[12]), t_tri=float(t[13])))
    return rows


def gnomonic(P, c):
    c = c / np.linalg.norm(c)
    a = np.array([0, 0, 1.0]) if abs(c[2]) < 0.9 else np.array([1.0, 0, 0])
    e1 = np.cross(a, c); e1 /= np.linalg.norm(e1)
    e2 = np.cross(c, e1)
    d = P @ c
    return np.stack([(P @ e1) / d, (P @ e2) / d], axis=1)


def sphArea(a, b, c):
    num = np.einsum('ij,ij->i', a, np.cross(b - a, c - a))
    den = 1 + np.einsum('ij,ij->i', a, b) + np.einsum('ij,ij->i', b, c) + np.einsum('ij,ij->i', c, a)
    return 2 * np.arctan2(num, den)


def drawPoly(ax, f, title, centre=None, zoom=None):
    V, T, B, st = readDump(f)
    P = xyz(V)
    nP = sum(n for b, n in B)
    c = P[:nP].mean(axis=0) if centre is None else centre
    X = gnomonic(P, c)
    if len(T):
        a = sphArea(P[T[:, 0]], P[T[:, 1]], P[T[:, 2]])
        s = np.sign(a.sum()) if a.sum() != 0 else 1
        inv = a * s < -1e-15
        polys = X[T]
        ax.add_collection(PolyCollection(polys[~inv], facecolor='#d5e8d4', edgecolor='#7f8c8d', linewidth=0.3))
        if inv.any():
            ax.add_collection(PolyCollection(polys[inv], facecolor='#f5b7b1', edgecolor='#c0392b', linewidth=0.6))
    # Boundary rings
    for b, n in B:
        idx = list(range(b, b + n)) + [b]
        ax.plot(X[idx, 0], X[idx, 1], '-', color='k', linewidth=0.8)
    # Missing ring edges
    E = set()
    for t in T:
        for k in range(3):
            i, j = t[k], t[(k + 1) % 3]
            E.add((min(i, j), max(i, j)))
    seg = []
    for b, n in B:
        for j in range(n):
            i1, i2 = b + j, b + (j + 1) % n
            if (min(i1, i2), max(i1, i2)) not in E:
                seg.append([X[i1], X[i2]])
    if seg:
        ax.add_collection(LineCollection(seg, colors='#e74c3c', linewidths=2.0))
    # Steiner points
    if len(V) > nP:
        ax.plot(X[nP:, 0], X[nP:, 1], 'o', color='#8e44ad', markersize=4)
    ax.set_aspect('equal')
    if zoom is not None:
        ax.set_xlim(zoom[0], zoom[1]); ax.set_ylim(zoom[2], zoom[3])
    else:
        ax.autoscale_view()
    ax.set_xticks([]); ax.set_yticks([])
    ax.set_title('%s\n%d triangles, %s' % (title, len(T), st), fontsize=8)
    return c


# Figure : bad rings, one row per case, one column per method
def figBadRings():
    cases = []
    for f in sorted(glob.glob(os.path.join(W, 'dumps', '*', 'cdt', 'poly_*.txt'))):
        case = f.split(os.sep)[-3]
        cases.append(case)
    meths = ['earcut', 'earcut-fix', 'triangle', 'cdt']
    for case in sorted(set(cases)):
        files = {m: glob.glob(os.path.join(W, 'dumps', case, m, 'poly_*.txt')) for m in meths}
        if not all(files.values()):
            continue
        fig, axs = plt.subplots(1, 4, figsize=(11, 3.2))
        c = None
        V, T, B, st = readDump(files['cdt'][0])
        P = xyz(V)
        c = P[:sum(n for b, n in B)].mean(axis=0)
        for ax, m in zip(axs, meths):
            drawPoly(ax, files[m][0], LABELS[m], centre=c)
        fig.suptitle(case.replace('_', ' '), fontsize=9)
        fig.tight_layout()
        fig.savefig(os.path.join(OUT, 'cmp_case_%s.pdf' % case))
        fig.savefig(os.path.join(OUT, 'cmp_case_%s.png' % case), dpi=120)
        plt.close(fig)


# Figure : time per ring against N
def figTimeVsN():
    meths = ['earcut', 'earcut-fix', 'triangle', 'cdt']
    fig, ax = plt.subplots(figsize=(6.5, 4.5))
    for m in meths:
        N, T = [], []
        for f in glob.glob(os.path.join(W, 'layers', 'gshhs_*_%s.txt' % m)):
            for r in readLayer(f):
                N.append(r['N']); T.append(r['t_proj'] + r['t_tri'])
        N, T = np.array(N), np.array(T)
        bins = np.unique(np.round(np.logspace(np.log10(4), np.log10(N.max() + 1), 25)))
        xb, yb = [], []
        for lo, hi in zip(bins[:-1], bins[1:]):
            k = (N >= lo) & (N < hi) & (T > 0)
            if k.sum() >= 3:
                xb.append(np.median(N[k])); yb.append(np.median(T[k]))
        big = N >= 3000
        ax.plot(N[big], T[big], '.', color=COLORS[m], markersize=3, alpha=0.6)
        ax.plot(xb, yb, '-', color=COLORS[m], label=LABELS[m])
    ax.set_xscale('log'); ax.set_yscale('log')
    ax.set_xlabel('number of vertices N')
    ax.set_ylabel('time per ring (s)')
    ax.grid(True, which='both', alpha=0.3)
    ax.legend(fontsize=8)
    ax.set_title('Time per ring, all GSHHS files (median per bin; dots: rings with N $\\geq$ 3000)', fontsize=9)
    fig.tight_layout()
    fig.savefig(os.path.join(OUT, 'cmp_time_vs_n.pdf'))
    fig.savefig(os.path.join(OUT, 'cmp_time_vs_n.png'), dpi=120)
    plt.close(fig)


# Figure : whole layer times, serial and parallel
def figLayerTimes():
    meths = ['earcut', 'earcut-fix', 'triangle', 'cdt']
    files = ['GSHHS_c_L1', 'GSHHS_i_L1', 'GSHHS_h_L1', 'GSHHS_h_L2', 'ne_110m_land', 'ne_50m_land', 'ne_10m_land']
    fig, axs = plt.subplots(1, 2, figsize=(11, 4), sharey=True)
    for ax, mode in zip(axs, ['serial', 'par']):
        x = np.arange(len(files))
        w = 0.2
        for i, m in enumerate(meths):
            v = []
            for f in files:
                # serial : timings1/ (OMP_NUM_THREADS=1) ; parallel : timings/*_par
                if mode == 'serial':
                    g = os.path.join(W, 'timings1', '%s_%s.txt' % (f, m))
                else:
                    g = os.path.join(W, 'timings', '%s_%s_%s.txt' % (f, m, mode))
                t = []
                if os.path.exists(g):
                    t = [float(mm.group(1)) for l in open(g) for mm in [re.search(r'wall (\S+)', l)] if mm]
                v.append(min(t) if t else np.nan)
            ax.bar(x + (i - 1.5) * w, v, w, color=COLORS[m], label=LABELS[m])
            for xi, vi in zip(x, v):
                if np.isnan(vi):
                    ax.text(xi + (i - 1.5) * w, 0.003, 'crash', rotation=90, fontsize=6, color=COLORS[m],
                            ha='center', va='bottom')
        ax.set_xticks(x)
        ax.set_xticklabels([f.replace('GSHHS_', '').replace('_land', '') for f in files], fontsize=8)
        ax.set_yscale('log')
        ax.grid(True, axis='y', which='both', alpha=0.3)
        ax.set_title('whole layer, %s' % ('1 thread' if mode == 'serial' else 'parallel, like Land.cc (14 threads)'), fontsize=9)
    axs[0].set_ylabel('wall time (s)')
    axs[0].legend(fontsize=8)
    fig.tight_layout()
    fig.savefig(os.path.join(OUT, 'cmp_layer_times.pdf'))
    fig.savefig(os.path.join(OUT, 'cmp_layer_times.png'), dpi=120)
    plt.close(fig)


figBadRings()
figTimeVsN()
figLayerTimes()
