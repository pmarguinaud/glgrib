GSHHS shapefiles: cleaning of self-intersecting rings
=====================================================

These files are the GSHHS shapefiles of GSHHG 2.3.7 (2017-06-15, the final release),
with a few rings repaired on 2026-11-26 by src/gshhsClean/gshhs_clean.

Why
---
A few rings cross themselves. Some crossings are in the data. Others appear when the
coordinates are converted to single precision, as glgrib does, which turns the points
next to the South Pole into tiny loops. Such rings cannot be triangulated correctly:
EarCut (glgrib's triangulation before 2026-11-26) leaves holes or fails, and the
constrained Delaunay triangulation (CDT) has to add points.

How
---
gshhs_clean finds the crossings exactly with CDT, using the coordinates glgrib sees
(degrees converted to single-precision radians), and repairs them with the nonzero
winding rule:
  - a loop outside the main ring is split off as a ring of its own;
  - a loop folded inside the main ring with the same orientation is dropped;
  - a reversed loop inside the main ring (a hole in the ring) is filled if small;
  - rings left with fewer than 3 points or a zero area are dropped.
No record is added or removed. The .dbf and .db files are unchanged.

Files changed (.shp and .shx only)
----------------------------------
  GSHHS_c_L5, GSHHS_h_L5, GSHHS_i_L5, GSHHS_l_L5 (Antarctica, ice front)
      Pole points: two loops of a few mm2 at the South Pole dropped.
  GSHHS_i_L5, GSHHS_l_L5
      Drygalski Ice Tongue (162.9E 75.4S): the end of a sea inlet, a reversed loop
      inside the ring, filled (59 km2 at i, 13 km2 at l); folds of about 2e4 m2 at the
      start of the ring dropped.
  GSHHS_c_L6, GSHHS_h_L6, GSHHS_i_L6, GSHHS_l_L6 (Antarctica, grounding line)
      Pole points: loops of a few mm2 split off, then dropped as degenerate.
  GSHHS_h_L6
      Two lobes split off as their own rings: 81.8W 79.1S (3.6e4 m2) and
      63.2W 82.3S (818 m2).
  GSHHS_h_L1
      Island in Maine (69.76W 43.81N): two folds dropped (5.6e3 and 7.0e3 m2).
  GSHHS_i_L1
      Same island: fold of 3.1e5 m2 dropped, lobe of 17 m2 split off.
  GSHHS_i_L2
      Lake at 8.63W 70.96N (Jan Mayen): lobe of 43 m2 split off.

The other files did not need any change and are the original ones.
All files pass "gshhs_clean in.shp" (check only) with no crossing left.

Reproduce
---------
  src/gshhsClean/clean-gshhs.sh OUTDIR    # cleans the original files, writes one .log
                                          # per file with every change and its location
The original files are in the git history (before 2026-11-26) and in the GSHHG 2.3.7
distribution (https://www.soest.hawaii.edu/pwessel/gshhg/).
More details: share/doc/earcut/gshhs-data-notes.md
