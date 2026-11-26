#!/usr/bin/env python3
#
# Summary tables for the comparison of triangulation methods
#
#   python3 summarize_comparison.py <work dir>
#
# Prints, for each layer file, the number of polygons that are not "ok" for each method
# (bad / error / empty, and ok-steiner separately), and the list of non-ok polygons.

import sys, os, glob, re, collections

W = sys.argv[1]
METHODS = ['earcut', 'earcut-fix', 'triangle', 'triangle-Y', 'cdt', 'cdt-noX', 'cdt-outer']

layers = collections.OrderedDict()
for f in sorted(glob.glob(os.path.join(W, 'layers', '*.txt'))):
    b = os.path.basename(f)[:-4]
    for m in sorted(METHODS, key=len, reverse=True):
        if b.endswith('_' + m):
            layers.setdefault(b[:-len(m) - 1], {})[m] = f
            break

def read(f):
    rows = {}
    for l in open(f):
        if l.startswith('#') or not l[:1].isdigit():
            continue
        t = l.split()
        rows[int(t[0])] = t
    return rows

def crashed(f):
    return any(l.startswith('# CRASH') for l in open(f))

def degenerate(rows):
    return sum(1 for t in rows.values() if t[6] == 'degenerate')

print('%-22s %8s %9s' % ('file', 'polys', 'vertices') + ''.join(' %11s' % m for m in METHODS))
details = []
for name, fm in layers.items():
    rows = {m: read(f) for m, f in fm.items()}
    any_ = rows[next(iter(rows))]
    nv = sum(int(t[1]) for t in any_.values())
    line = '%-22s %8d %9d' % (name, len(any_), nv)
    ndeg = max(degenerate(r) for r in rows.values())
    for m in METHODS:
        if m not in rows:
            line += ' %11s' % '-'
            continue
        c = collections.Counter(t[6] for t in rows[m].values())
        bad = c['bad'] + c['error'] + c['empty']
        if crashed(fm[m]):
            line += ' %11s' % 'CRASH'
        else:
            line += ' %7d/%-3d' % (bad, c['ok-steiner'])
    print(line + ('   (%d degenerate)' % ndeg if ndeg else ''))
    ranks = sorted(set(r for m in rows for r, t in rows[m].items() if t[6] not in ('ok', 'degenerate')))
    for r in ranks:
        details.append((name, r, {m: rows[m][r] for m in rows if r in rows[m]}))

print()
print('Cells : number of polygons bad+error+empty / number ok with Steiner points ; CRASH : the method aborted')
print('Degenerate : rings of zero area (< 1e-16 sr), not counted as failures')

# Inverted (overlapping) triangles : number of polygons with inverted triangles, largest inverted
# area in one polygon (m^2), and missing area (incomplete or wrong triangulations, km^2)
R2 = 6371.0e3 ** 2
print()
print('Inverted triangles : polygons with inverted triangles / largest inverted area in one polygon (m^2)')
print('%-22s' % 'file' + ''.join(' %16s' % m for m in METHODS))
for name, fm in layers.items():
    line = '%-22s' % name
    for m in METHODS:
        if m not in fm:
            line += ' %16s' % '-'
            continue
        rows = read(fm[m]).values()
        rows = [t for t in rows if len(t) > 16]
        n = sum(1 for t in rows if int(t[9]) > 0)
        mx = max([float(t[16]) * R2 for t in rows if int(t[9]) > 0] or [0.0])
        line += ' %5d/%-10.3g' % (n, mx)
    print(line)
print()
print('Non-ok polygons : file rank N nholes | per method : status ntri/expected steiner area_err overlap inverted missing')
for name, r, d in details:
    t0 = next(iter(d.values()))
    print('%s %d N=%s holes=%s' % (name, r, t0[1], t0[2]))
    for m in METHODS:
        if m in d:
            t = d[m]
            print('    %-11s %-10s %s/%s st=%s aerr=%s ovl=%s inv=%s miss=%s' % (m, t[6], t[3], t[4], t[5], t[7], t[8], t[9], t[10]))
