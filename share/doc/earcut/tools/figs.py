"""Figures for the EarCut document.

Usage : python3 figs.py <data dir> <output dir>

<data dir> holds the dumps written by the EARCUT_DUMP instrumentation :
  rd_<res>_<mode>_fix<0|1>/ring_<rank>.txt : one ring, pass by pass
  st_<res>_<mode>_fix<0|1>/stats.txt      : one line per ring of a layer
mode = omp (OpenMP pools for rings >= 300 points, as in Land.cc) or noomp
fix0 = code as is (xyz2 = R xyz1), fix1 = transposed rotation (xyz2 = R^T xyz1)
"""

import sys, os
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.collections import PolyCollection
from ring import Ring

D, O = sys.argv[1], sys.argv[2]
os.makedirs(O, exist_ok=True)

plt.rcParams.update({'font.size': 8, 'axes.titlesize': 8.5, 'figure.dpi': 150,
                     'savefig.bbox': 'tight', 'axes.spines.top': False,
                     'axes.spines.right': False, 'legend.frameon': False})
CM = plt.get_cmap('viridis')
EDGE = '#222222'
RED = '#d62728'
FILL = '#dde7f3'
FILLEDGE = '#7f9cc0'


def save(fig, name):
    fig.savefig(os.path.join(O, name + '.pdf'))
    fig.savefig(os.path.join(O, name + '.png'))
    plt.close(fig)


def xyz(lon, lat):
    lo, la = np.radians(lon), np.radians(lat)
    return np.stack([np.cos(lo) * np.cos(la), np.sin(lo) * np.cos(la), np.sin(la)], 1)


class Gnomonic:
    """Gnomonic projection centred on the mean point of a ring : great circles
    (the ring edges, the triangle edges) are straight lines."""

    def __init__(self, r):
        X = xyz(r.lon, r.lat)
        c = X.mean(0); c /= np.linalg.norm(c)
        e = np.cross([0, 0, 1], c); e /= np.linalg.norm(e)
        self.c, self.e, self.n = c, e, np.cross(c, e)

    def __call__(self, r):
        X = xyz(r.lon, r.lat)
        d = X @ self.c
        return np.degrees(X @ self.e / d), np.degrees(X @ self.n / d)


def tris(r, x, y, kmin=0, kmax=None):
    t = r.tri[r.tri[:, 0] >= kmin]
    if kmax is not None:
        t = t[t[:, 0] <= kmax]
    v = np.stack([np.stack([x[t[:, j]], y[t[:, j]]], 1) for j in (2, 1, 3)], 1)
    return v, t[:, 0]


def ring_line(ax, x, y, idx=None, **kw):
    idx = np.arange(len(x)) if idx is None else np.asarray(idx)
    ax.plot(np.append(x[idx], x[idx[0]]), np.append(y[idx], y[idx[0]]), **kw)


def clean(ax, title=None):
    ax.set_aspect('equal')
    ax.set_xticks([]); ax.set_yticks([])
    for s in ax.spines.values():
        s.set_visible(False)
    if title:
        ax.set_title(title)


def angles(r, idx):
    """Angle of node_t::getAngle for the ring made of vertices idx (in order)."""
    X = xyz(r.rlon, r.rlat)[idx]
    P, N = np.roll(X, 1, 0), np.roll(X, -1, 0)
    ji = np.cross(X, P - X)
    jk = np.cross(X, N - X)
    return np.arctan2(np.sum(X * np.cross(ji, jk), 1), np.sum(ji * jk, 1))


def stats(path):
    out = []
    for line in open(path):
        w = line.split()
        rank, n, omp, ntri = map(int, w[:4])
        c = np.array([list(map(int, s.split(':'))) for s in w[4:]]).reshape(-1, 2)
        out.append((rank, n, omp, ntri, c))
    return out


def npasses(s):
    """Number of passes that cut at least one ear."""
    rem = np.concatenate([[s[1]], s[4][:, 0]])
    return int(np.sum(np.diff(rem) < 0))


# ---------------------------------------------------------------------------
# Fig. : passes on a single ring (Australia, crude, 169 points)

r = Ring(f'{D}/rd_c_noomp_fix1/ring_2230.txt')
x, y = Gnomonic(r)(r)
npass = r.passes[-1, 0] + 1
show = [-1, 0, 1, 2, 4, 7]
fig, axs = plt.subplots(2, 3, figsize=(7.2, 4.6))
for ax, k in zip(axs.flat, show):
    alive = r.alive(k)
    if k >= 0:
        v, p = tris(r, x, y, kmax=k)
        ax.add_collection(PolyCollection(v, array=p, cmap=CM, clim=(0, npass - 1),
                                         edgecolors='white', linewidths=0.3))
    ring_line(ax, x, y, color='#bbbbbb', lw=0.5)
    idx = np.nonzero(alive)[0]
    ring_line(ax, x, y, idx, color=EDGE, lw=0.8)
    refl = angles(r, idx) < 0
    ax.plot(x[idx][refl], y[idx][refl], 'o', ms=2.0, color=RED)
    ax.autoscale_view()
    nleft = r.n if k < 0 else r.passes[k, 1]
    clean(ax)
    ax.set_title(('initial ring' if k < 0 else f'after pass {k + 1}') +
                 f'\n{nleft} vertices, {refl.sum()} reflex')
sm = plt.cm.ScalarMappable(cmap=CM, norm=plt.Normalize(0.5, npass + 0.5))
fig.colorbar(sm, ax=axs, shrink=0.6, label='pass in which the triangle was cut',
             ticks=range(1, npass + 1, 2))
save(fig, 'fig_passes_australia')

# ---------------------------------------------------------------------------
# Fig. : full triangulations coloured by pass (Africa, Eurasia)

fig, axs = plt.subplots(1, 2, figsize=(7.2, 3.4), gridspec_kw={'width_ratios': [1, 1.6]})
for ax, f, name in zip(axs, ['rd_c_noomp_fix1/ring_1031.txt', 'rd_c_noomp_fix1/ring_0.txt'],
                       ['Africa', 'Eurasia']):
    r = Ring(f'{D}/{f}')
    x, y = Gnomonic(r)(r)
    v, p = tris(r, x, y)
    npass = r.passes[-1, 0] + 1
    ax.add_collection(PolyCollection(v, array=p / (npass - 1), cmap=CM, clim=(0, 1),
                                     edgecolors='white', linewidths=0.15))
    ring_line(ax, x, y, color=EDGE, lw=0.4)
    ax.autoscale_view()
    clean(ax, f'{name}: {r.n} vertices\n{len(r.tri)} triangles, {npass - 1} passes')
sm = plt.cm.ScalarMappable(cmap=CM, norm=plt.Normalize(0, 1))
fig.colorbar(sm, ax=axs, shrink=0.7, label='pass / last pass')
save(fig, 'fig_triangulation_by_pass')

# ---------------------------------------------------------------------------
# Fig. : convergence of the largest ring at three resolutions

fig, axs = plt.subplots(1, 2, figsize=(7.2, 3.0), gridspec_kw={'wspace': 0.3})
cols = {'c': '#1f77b4', 'i': '#ff7f0e', 'h': '#2ca02c'}
for rr, lab in [('c', 'crude'), ('i', 'intermediate'), ('h', 'high')]:
    s = max(stats(f'{D}/st_{rr}_noomp_fix1/stats.txt'), key=lambda s: s[1])
    rank, n, omp, ntri, c = s
    rem = np.concatenate([[n], c[:, 0]])
    keep = np.concatenate([[True], np.diff(rem) < 0])
    rem = rem[keep]
    k = np.arange(len(rem))
    axs[0].semilogy(k, rem, '-', color=cols[rr], label=f'{lab}, N = {n}')
    refl = np.concatenate([[np.nan], c[:, 1]])[keep]
    axs[0].semilogy(k, np.where(refl > 0, refl, np.nan), ':', color=cols[rr])
    frac = 1 - rem[1:] / rem[:-1]
    sel = rem[:-1] >= 20
    axs[1].plot(k[1:][sel], frac[sel], '-', color=cols[rr], label=f'{lab}, N = {n}')
axs[0].set_xlabel('pass'); axs[0].set_ylabel('vertices')
axs[0].set_title('Eurasia: vertices left (solid),\nreflex vertices left (dotted)')
axs[0].legend(fontsize=7)
axs[1].set_xlabel('pass'); axs[1].set_ylabel('fraction cut')
axs[1].set_title('Fraction of the remaining vertices\ncut by each pass (while >= 20 remain)')
axs[1].set_ylim(0, 0.45)
save(fig, 'fig_convergence')

# ---------------------------------------------------------------------------
# Fig. : number of passes as a function of ring size

fig, axs = plt.subplots(1, 2, figsize=(7.2, 3.0), sharey=True)
for ax, rr, lab in [(axs[0], 'h', 'GSHHS high, level 1 (land)'),
                    (axs[1], 'hL2', 'GSHHS high, level 2 (lakes, rivers)')]:
    for mode, col, ml in [('noomp', '#1f77b4', 'serial'), ('omp', '#ff7f0e', 'OpenMP pools (N >= 300)')]:
        S = stats(f'{D}/st_{rr}_{mode}_fix1/stats.txt')
        n = np.array([s[1] for s in S]); p = np.array([npasses(s) for s in S])
        sel = (n >= 4) if mode == 'noomp' else (n >= 300)
        ax.scatter(n[sel], p[sel], s=2, alpha=0.3, color=col, label=ml, rasterized=True)
        if mode == 'noomp':
            edges = np.logspace(np.log10(4), np.log10(n.max() + 1), 16)
            ib = np.digitize(n[sel], edges)
            med = [(np.sqrt(edges[b - 1] * edges[b]), np.median(p[sel][ib == b]))
                   for b in range(1, len(edges)) if np.sum(ib == b) >= 5]
            med = np.array(med)
            ax.plot(med[:, 0], med[:, 1], 'k-', lw=1, label='serial, median')
    nn = np.logspace(np.log10(4), 5.2, 50)
    ax.plot(nn, np.log(nn) / np.log(1 / 0.7), 'k:', lw=0.8, label='log(N) / log(1/0.7)')
    ax.set_xscale('log'); ax.set_yscale('log')
    ax.set_xlabel('ring size N (vertices)')
    ax.set_title(lab)
    ax.legend(fontsize=6.5, markerscale=4, loc='upper left')
axs[0].set_ylabel('passes that cut at least one ear')
save(fig, 'fig_passes_vs_n')

# ---------------------------------------------------------------------------
# Fig. : OpenMP partition of the first pass (Eurasia crude, 1003 points)

r = Ring(f'{D}/rd_c_omp_fix1/ring_0.txt')
x, y = Gnomonic(r)(r)
n = r.n
chunk = n // 8
t0 = r.tri[r.tri[:, 0] == 0]
pool = (((t0[:, 1] - 10) % n) // chunk).clip(0, 7)
v = np.stack([np.stack([x[t0[:, j]], y[t0[:, j]]], 1) for j in (2, 1, 3)], 1)
fig, ax = plt.subplots(figsize=(6.0, 3.6))
ring_line(ax, x, y, color='#999999', lw=0.4)
ax.add_collection(PolyCollection(v, array=pool, cmap=plt.get_cmap('tab10'), clim=(0, 9),
                                 edgecolors='none'))
for c in range(8):
    j = (10 + c * chunk) % n
    ax.plot(x[j], y[j], 'k>', ms=4)
    ax.annotate(str(c), (x[j], y[j]), fontsize=7, xytext=(4, 2), textcoords='offset points')
ax.autoscale_view()
clean(ax, f'First pass on Eurasia (crude) with OpenMP: {len(t0)} ears cut by 8 pools\n'
          f'(colour = pool, arrow = first vertex of the pool)')
save(fig, 'fig_openmp_pools')

# ---------------------------------------------------------------------------
# Fig. : rotated frames, current code and transposed matrix

fig, axs = plt.subplots(2, 3, figsize=(7.2, 3.6))
for col, (f, name) in enumerate([('rd_c_noomp_fix{}/ring_0.txt', 'Eurasia (crude)'),
                                 ('rd_c_noomp_fix{}/ring_1252.txt', 'North America (crude)'),
                                 ('rd_i_noomp_fix{}/ring_102218.txt', 'Honshu (intermediate)')]):
    for row, fix in enumerate([0, 1]):
        r = Ring(f'{D}/' + f.format(fix))
        ax = axs[row, col]
        ax.axhline(0, color='#cccccc', lw=0.5); ax.axvline(0, color='#cccccc', lw=0.5)
        ax.plot(r.rlon, r.rlat, '.', ms=0.6, color=EDGE)
        ax.set_xlim(-180, 180); ax.set_ylim(-90, 90)
        ax.set_xticks([-180, -90, 0, 90, 180]); ax.set_yticks([-90, 0, 90])
        ax.set_aspect('equal')
        if row == 0:
            ax.set_title(name)
axs[0, 0].set_ylabel('xyz2 = R xyz1\n(current code)')
axs[1, 0].set_ylabel('xyz2 = R$^T$ xyz1')
save(fig, 'fig_rotation')

# ---------------------------------------------------------------------------
# Fig. : failure on Honshu (intermediate resolution, OpenMP, current rotation)

r0 = Ring(f'{D}/rd_i_omp_fix0/ring_102218.txt')
r1 = Ring(f'{D}/rd_i_omp_fix1/ring_102218.txt')
G = Gnomonic(r0)
x, y = G(r0)


def window(ax, ids, pad):
    ax.set_xlim(x[ids].min() - pad, x[ids].max() + pad)
    ax.set_ylim(y[ids].min() - pad, y[ids].max() + pad)


def label(ax, ids):
    for i in ids:
        ax.annotate(str(i), (x[i], y[i]), fontsize=6.5, xytext=(2, 2), textcoords='offset points')


fig, axs = plt.subplots(2, 2, figsize=(6.4, 5.6))
axs = axs.flatten()
ring_line(axs[0], x, y, color=EDGE, lw=0.4)
axs[0].plot(x[[786, 834]], y[[786, 834]], 's', ms=8, mfc='none', color=RED)
axs[0].autoscale_view(); clean(axs[0], 'Honshu, 1541 vertices')

# (b) the bad ear, cut in pass 4
ax = axs[1]
k = 3
v, _ = tris(r0, x, y, kmax=k - 1)
ax.add_collection(PolyCollection(v, facecolors=FILL, edgecolors=FILLEDGE, linewidths=0.3))
ring_line(ax, x, y, color=EDGE, lw=0.5)
bad = np.array([[x[779], y[779]], [x[786], y[786]], [x[791], y[791]]])
ax.add_collection(PolyCollection([bad], facecolors='#f7c6c7', edgecolors=RED, linewidths=1.0))
ax.plot(x[792], y[792], 'o', ms=4, color=RED)
ids = [779, 786, 791, 792, 793]
label(ax, ids); window(ax, ids, 0.02)
clean(ax, 'pass 4: ear (779, 786, 791)\nis cut although 792 is inside')

# (c) stuck state
ax = axs[2]
v, _ = tris(r0, x, y)
ax.add_collection(PolyCollection(v, facecolors=FILL, edgecolors=FILLEDGE, linewidths=0.3))
ring_line(ax, x, y, color=EDGE, lw=0.5)
idx = np.nonzero(r0.alive(r0.passes[-1, 0]))[0]
ring_line(ax, x, y, idx, color=RED, lw=1.2)
ids = [786, 791, 792, 793, 834]
label(ax, idx); window(ax, ids, 0.03)
clean(ax, f'end: {len(idx)} vertices left,\nall reflex (inverted remainder)')

# (d) with R^T
ax = axs[3]
v, _ = tris(r1, x, y)
ax.add_collection(PolyCollection(v, facecolors=FILL, edgecolors=FILLEDGE, linewidths=0.3))
ring_line(ax, x, y, color=EDGE, lw=0.5)
window(ax, ids, 0.03)
clean(ax, 'with R$^T$: 1539 triangles,\ncomplete')
save(fig, 'fig_failure_honshu')

# ---------------------------------------------------------------------------
# Fig. : river ring (Ob - Irtysh, lakes layer, intermediate) : slow convergence

rs = Ring(f'{D}/rd_iL2_noomp_fix1/ring_87722.txt')
ro = Ring(f'{D}/rd_iL2_omp_fix1/ring_87722.txt')
G = Gnomonic(rs)
x, y = G(rs)
fig, axs = plt.subplots(1, 3, figsize=(7.4, 2.8), gridspec_kw={'width_ratios': [1, 1.1, 1.3], 'wspace': 0.35})
ring_line(axs[0], x, y, color=EDGE, lw=0.5)
zx, zy = np.median(x[315:325]), np.median(y[315:325])
w = 0.3
axs[0].add_patch(plt.Rectangle((zx - w, zy - w), 2 * w, 2 * w, fill=False, color=RED, lw=0.8))
axs[0].autoscale_view(); clean(axs[0], f'Ob - Irtysh, {rs.n} vertices')
ax = axs[1]
npass = rs.passes[-1, 0] + 1
v, p = tris(rs, x, y)
ax.add_collection(PolyCollection(v, array=p, cmap=CM, clim=(0, npass - 1),
                                 edgecolors='white', linewidths=0.1))
ring_line(ax, x, y, color=EDGE, lw=0.4)
ax.set_xlim(zx - w, zx + w); ax.set_ylim(zy - w, zy + w)
clean(ax, 'zoom, serial: colour = pass')
ax = axs[2]
for rr, col, lab in [(rs, '#1f77b4', 'serial'), (ro, '#ff7f0e', 'OpenMP pools')]:
    rem = np.concatenate([[rr.n], rr.passes[:, 1]])
    ax.semilogy(np.arange(len(rem)), rem, color=col, label=lab)
ax.axhline(ro.passes[-1, 1], color='#ff7f0e', lw=0.5, ls=':')
ax.set_xlabel('pass'); ax.set_ylabel('vertices left')
ax.legend(fontsize=7)
ax.set_title(f'serial: {rs.passes[-1, 0]} passes; OpenMP stops\nwith {ro.passes[-1, 1]} vertices left')
save(fig, 'fig_river')

# ---------------------------------------------------------------------------
# Fig. : self-intersecting ring (cannot be triangulated)

r = Ring(f'{D}/rd_i_noomp_fix1/ring_194020.txt')
x, y = Gnomonic(r)(r)
fig, axs = plt.subplots(1, 2, figsize=(5.2, 2.8))
ring_line(axs[0], x, y, color=EDGE, lw=0.8)
for i in range(r.n):
    axs[0].annotate(str(i), (x[i], y[i]), fontsize=6, xytext=(2, 2), textcoords='offset points')
axs[0].autoscale_view(); clean(axs[0], f'{r.n} vertices; edges (1,2)-(18,0)\nand (14,15)-(16,17) cross')
k = r.passes[-1, 0]
v, p = tris(r, x, y)
axs[1].add_collection(PolyCollection(v, facecolors=FILL, edgecolors=FILLEDGE, linewidths=0.4))
ring_line(axs[1], x, y, color=EDGE, lw=0.6)
ring_line(axs[1], x, y, np.nonzero(r.alive(k))[0], color=RED, lw=1.2)
axs[1].autoscale_view(); clean(axs[1], f'{len(r.tri)} triangles, {r.passes[-1, 1]} vertices left')
save(fig, 'fig_selfintersect')

print('done')
