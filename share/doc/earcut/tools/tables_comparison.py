#!/usr/bin/env python3
#
# LaTeX tables for the comparison report, generated from the work directory
#
#   python3 tables_comparison.py <work dir> <output dir>

import sys, os, glob, re, collections
import numpy as np

W, OUT = sys.argv[1], sys.argv[2]
os.makedirs(OUT, exist_ok=True)
R2 = 6371.0e3 ** 2

METHODS = ['earcut', 'earcut-fix', 'triangle', 'triangle-Y', 'cdt', 'cdt-noX', 'cdt-outer']
MAIN = ['earcut', 'earcut-fix', 'triangle', 'cdt']

FILES = [('gshhs_c_L1', 'GSHHS c L1'), ('gshhs_c_L2', 'GSHHS c L2'), ('gshhs_c_L3', 'GSHHS c L3'),
         ('gshhs_c_L5', 'GSHHS c L5'), ('gshhs_c_L6', 'GSHHS c L6'),
         ('gshhs_i_L1', 'GSHHS i L1'), ('gshhs_i_L2', 'GSHHS i L2'), ('gshhs_i_L3', 'GSHHS i L3'),
         ('gshhs_i_L4', 'GSHHS i L4'), ('gshhs_i_L5', 'GSHHS i L5'), ('gshhs_i_L6', 'GSHHS i L6'),
         ('gshhs_h_L1', 'GSHHS h L1'), ('gshhs_h_L2', 'GSHHS h L2'), ('gshhs_h_L3', 'GSHHS h L3'),
         ('gshhs_h_L4', 'GSHHS h L4'), ('gshhs_h_L5', 'GSHHS h L5'), ('gshhs_h_L6', 'GSHHS h L6'),
         ('ne511_110m_land', 'NE 5.1.1 110m land'), ('ne511_50m_land', 'NE 5.1.1 50m land'),
         ('ne511_10m_land', 'NE 5.1.1 10m land'), ('ne511_110m_lakes', 'NE 5.1.1 110m lakes'),
         ('ne511_50m_lakes', 'NE 5.1.1 50m lakes'), ('ne511_10m_lakes', 'NE 5.1.1 10m lakes'),
         ('ne301_110m_land', 'NE 3.0.1 110m land'), ('ne301_50m_land', 'NE 3.0.1 50m land'),
         ('ne301_10m_land', 'NE 3.0.1 10m land')]


def read(f):
    rows, crash = {}, False
    for l in open(f):
        if l.startswith('# CRASH'):
            crash = True
        if not l[:1].isdigit():
            continue
        t = l.split()
        rows[int(t[0])] = t
    return rows, crash


def layer(name, m):
    f = os.path.join(W, 'layers', '%s_%s.txt' % (name, m))
    return read(f) if os.path.exists(f) else ({}, False)


# Table 1 : robustness
with open(os.path.join(OUT, 'tab_robustness.tex'), 'w') as fp:
    fp.write('\\begin{tabular}{lrrrrrrrrr}\n\\toprule\n')
    fp.write('file & polygons & holes & vertices & degen. & EarCut & EarCut fixed & Triangle & CDT & CDT no-X \\\\\n\\midrule\n')
    for name, label in FILES:
        ref, _ = layer(name, 'cdt')
        if not ref:
            continue
        nv = sum(int(t[1]) for t in ref.values())
        nh = sum(int(t[2]) for t in ref.values())
        ndeg = sum(1 for t in ref.values() if t[6] == 'degenerate')
        cells = []
        for m in MAIN + ['cdt-noX']:
            rows, crash = layer(name, m)
            if crash:
                cells.append('crash')
                continue
            c = collections.Counter(t[6] for t in rows.values())
            bad = c['bad'] + c['error'] + c['empty']
            st = c['ok-steiner']
            cell = '%d' % bad if bad else '0'
            if st:
                cell += ' (+%d)' % st
            if bad:
                cell = '\\textbf{%s}' % cell
            cells.append(cell)
        fp.write('%s & %d & %d & %d & %s & %s \\\\\n' % (label, len(ref), nh, nv, ndeg if ndeg else '', ' & '.join(cells)))
        if name in ('gshhs_c_L6', 'gshhs_i_L6', 'gshhs_h_L6', 'ne511_10m_lakes'):
            fp.write('\\midrule\n')
    fp.write('\\bottomrule\n\\end{tabular}\n')

# Table 2 : inverted triangles (EarCut only ; Triangle and CDT have none)
with open(os.path.join(OUT, 'tab_inverted.tex'), 'w') as fp:
    fp.write('\\begin{tabular}{lrrrr}\n\\toprule\n')
    fp.write(' & \\multicolumn{2}{c}{EarCut (glgrib)} & \\multicolumn{2}{c}{EarCut fixed} \\\\\n')
    fp.write('file & polygons & largest (m$^2$) & polygons & largest (m$^2$) \\\\\n\\midrule\n')
    tot = {}
    for name, label in FILES:
        cells, anyinv = [], False
        for m in ['earcut', 'earcut-fix']:
            rows, crash = layer(name, m)
            if crash:
                cells += ['crash', '']
                continue
            inv = [float(t[16]) * R2 for t in rows.values() if int(t[9]) > 0]
            anyinv = anyinv or bool(inv)
            cells += ['%d' % len(inv), '%.0f' % max(inv) if inv else '']
        if anyinv:
            fp.write('%s & %s \\\\\n' % (label, ' & '.join(cells)))
    fp.write('\\bottomrule\n\\end{tabular}\n')
    # check that Triangle and CDT never have inverted triangles
    for name, label in FILES:
        for m in ['triangle', 'triangle-Y', 'cdt', 'cdt-noX', 'cdt-outer']:
            rows, crash = layer(name, m)
            n = sum(1 for t in rows.values() if int(t[9]) > 0)
            if n:
                print('WARNING inverted triangles for', name, m, n)

# Table 3 : timings
def best(f):
    if not os.path.exists(f):
        return None
    t = [float(mm.group(1)) for l in open(f) for mm in [re.search(r'wall (\S+)', l)] if mm]
    return min(t) if t else None

TF = [('GSHHS_c_L1', 'GSHHS c L1'), ('GSHHS_i_L1', 'GSHHS i L1'), ('GSHHS_h_L1', 'GSHHS h L1'),
      ('GSHHS_h_L2', 'GSHHS h L2'), ('ne_110m_land', 'NE 110m land'), ('ne_50m_land', 'NE 50m land'),
      ('ne_10m_land', 'NE 10m land')]
with open(os.path.join(OUT, 'tab_timings.tex'), 'w') as fp:
    fp.write('\\begin{tabular}{lrrrrrrrr}\n\\toprule\n')
    fp.write(' & \\multicolumn{4}{c}{1 thread} & \\multicolumn{4}{c}{parallel (14 threads)} \\\\\n')
    fp.write('\\cmidrule(lr){2-5}\\cmidrule(lr){6-9}\n')
    fp.write('file & EarCut & fixed & Triangle & CDT & EarCut & fixed & Triangle & CDT \\\\\n\\midrule\n')
    for f, label in TF:
        c1 = [best(os.path.join(W, 'timings1', '%s_%s.txt' % (f, m))) for m in MAIN]
        cp = [best(os.path.join(W, 'timings', '%s_%s_par.txt' % (f, m))) for m in MAIN]
        def fmt(v, row):
            if v is None:
                return 'crash'
            s = '%.3f' % v
            vals = [x for x in row if x is not None]
            return '\\textbf{%s}' % s if v == min(vals) else s
        fp.write('%s & %s & %s \\\\\n' % (label, ' & '.join(fmt(v, c1) for v in c1), ' & '.join(fmt(v, cp) for v in cp)))
    fp.write('\\bottomrule\n\\end{tabular}\n')

print('tables written to', OUT)
