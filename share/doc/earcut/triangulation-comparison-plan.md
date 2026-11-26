# Plan: compare EarCut with constrained Delaunay triangulation (Triangle, CDT)

Written 2026-11-25. Status: **done on 2026-11-25**, except the optional §6.3 (synthetic cases) and §9
(trial inside glgrib). Report: `triangulation-comparison.pdf`; tools: `tools/tri_driver.cc`,
`tools/build-tri-driver.sh`, `tools/make-comparison.sh`, `tools/make-comparison-dumps.sh`,
`tools/summarize_comparison.py`, `tools/tables_comparison.py`, `tools/figs_comparison.py`,
`tools/quality_comparison.py`.
Related: `earcut.pdf` (how EarCut works and its defects), `gshhs-data-notes.md` (bad rings in GSHHS).

## 1. Goal

Answer three questions:

1. **Robustness:** can constrained Delaunay triangulation (CDT) handle the GSHHS rings that
   defeat EarCut: self-crossings, near-duplicate points, the pole, and long thin rivers?
2. **Performance:** how fast is it compared with EarCut, per ring and for whole layers,
   serial and parallel?
3. **Fit with glgrib:** what would replacing or backing up EarCut cost? This covers new
   vertices, projection to the plane, licence, dependencies and thread safety.

## 2. Candidates

| name | what it is | licence | role in the test |
|---|---|---|---|
| EarCut (glgrib as is) | `src/glGrib/EarCut.cc` | GPLv3 (glgrib) | reference |
| EarCut fixed | same, with `transpose(R)` and serial passes (`GLGRIB_EARCUT_FIXROT`, `GLGRIB_EARCUT_NOOMP` from the instrumentation patch) | GPLv3 | shows how much fixing the two bugs alone brings |
| Triangle | J. Shewchuk's C program/library (`triangle.c`, v1.6), constrained Delaunay | non-free: research and private use only, not sellable; **cannot be shipped with glgrib** | benchmark of a mature, robust CDT |
| CDT | artem-ogre/CDT, header-only C++ | MPL-2.0 (can be combined with GPLv3) | realistic candidate for glgrib |

The algorithm (constrained Delaunay) is public; only the implementations are licensed.

Getting the code (no sudo, nothing installed system-wide):
- Triangle: download `triangle.zip` from netlib (https://www.netlib.org/voronoi/triangle.zip),
  compile `triangle.c` with `-DTRILIBRARY -DANSI_DECLARATORS`. Not using the Ubuntu
  `libtriangle-dev` package.
- CDT: download a tagged release from GitHub and record the version used. Header-only, so no
  build step.
- Put both under `share/doc/earcut/tools/ext/` and do not commit them (or keep them in the
  scratchpad). Record the download URLs and checksums in the report.

## 2b. Data sets

### GSHHS (what glgrib uses now)

GSHHG 2.3.7, in `share/glgrib/coastlines/shp`, resolutions c, i, h (l only for L2–L6; f not
present). Known bad rings are listed in §6.1.

### Natural Earth (added 2026-11-25)

Natural Earth is the coastline data set used by ECMWF Magics: `share/magics/{10m,50m,110m}`,
version 3.0.1, files `ne_*_land` and `ne_*_ocean`. It is public domain and still
maintained, so it is a possible replacement for GSHHS.

| Natural Earth | scale | Magics setting | rough GSHHS counterpart (to be checked) |
|---|---|---|---|
| `ne_110m_land` | 1:110 million | `low` | c |
| `ne_50m_land` | 1:50 million | `medium` | l / i |
| `ne_10m_land` | 1:10 million | `high`, `full` | i / h |

Lakes are separate files in Natural Earth (`ne_*_lakes`), like GSHHS L2.

**Getting it:** download the land and lakes shapefiles of the latest release from
naturalearthdata.com (or its GitHub repository, nvkelso/natural-earth-vector). Record the
version. Also test Magics' copy (3.0.1) to see whether newer releases fixed problems.

**Differences from GSHHS that the driver must handle:**

1. **Holes:** Natural Earth polygons can have inner rings (holes) as extra parts of a
   shapefile record. In the shapefile convention, outer rings are clockwise and holes are
   counter-clockwise. glgrib's loader treats every part as a separate ring to fill, so
   holes would be filled as land.
   - The driver must classify the parts by orientation.
   - Count how many polygons have holes.
   - For EarCut, either skip the holes (and report them), or join each hole to its outer
     ring with a bridge edge, as other ear-cutting codes do.
   - Triangle and CDT accept holes directly.
2. **Antarctica:** in Natural Earth it is probably closed along latitude −90° (a run of
   points at the pole from −180° to +180°); this is not checked yet. If so, it is the same
   kind of problem as h_L6 17477. Check how each method handles it.
3. **Dateline:** polygons may be cut at ±180° (Chukotka, Fiji). This gives seams with
   duplicate edges but does not break the triangulation. Note it for the visual
   comparison.
4. **Attributes:** Natural Earth has no GSHHS-style levels, and the `.dbf` fields differ.
   The driver only needs the geometry.

**Data-quality checks:** run the same checks as for GSHHS (`gshhs-data-notes.md`):
self-crossings, near-duplicate points and spikes, per file. Compare the counts with
GSHHS. Is Natural Earth cleaner?

## 3. The main technical issue: the sphere

Triangle and CDT work in the plane; GSHHS rings live on the sphere. Each ring must be
projected to a plane, triangulated, and the triangle indices mapped back to the original
vertices.

- **Projection:** stereographic projection seen from the antipode of the ring's centre,
  using the same centre/rotation as EarCut, but corrected (`transpose(R)`).
  - Why stereographic: it maps circles to circles. A Delaunay triangulation in the plane is
    therefore also Delaunay on the sphere, and the projection works for rings larger than a
    hemisphere (Eurasia).
  - Its drawback: great-circle edges become circular arcs, not straight segments. With
    GSHHS's short edges this should not matter, but check it (step 5.3).
  - The gnomonic projection (straight edges) is the alternative for rings smaller than a
    hemisphere. It fails for Eurasia and Antarctica.
- **Antarctica (L5, L6):** the rings contain the South Pole, so the projection is centred
  on it. The h_L6 ring that passes through the pole twice is a special test case.
- **New vertices (Steiner points):** both libraries can add vertices, for example where
  edges cross. glgrib's index buffer only refers to the original vertices.
  - Count the added vertices and record them per ring.
  - Run each library in two modes:
    - (a) no new vertices allowed: Triangle `-Y`, no `-q`; CDT `IntersectingConstraintEdges::NotAllowed`. Crossings then make the ring fail;
    - (b) crossings resolved by adding vertices: Triangle default `-p`; CDT `TryResolve`.
  - In glgrib, mode (b) would mean appending the new vertices to the vertex buffer.
- **Outside triangles:** Triangle `-p` without `-c` removes the triangles outside the ring.
  CDT has `eraseOuterTriangles`, or `eraseOuterTrianglesAndHoles`, which uses even–odd
  depth and so gives a defined answer for self-crossing rings. Test both CDT variants.
- **Duplicates:** Triangle skips duplicate vertices with a warning. For CDT, call
  `RemoveDuplicatesAndRemapEdges` first. Also test what happens to near-duplicates
  (i_L2 81960, 1e-6 rad apart) in single and double precision.

## 4. Test harness

Extend `tools/earcut_driver.cc` into `tools/tri_driver.cc` with one method argument:

```
tri_driver file.shp METHOD rank...|all|min=N [options]
METHOD = earcut | earcut-fix | triangle | triangle-Y | cdt | cdt-noX
```

- It reads the shapefile exactly as glgrib does: `len = part vertices − 1`, the same rings.
- For each ring it writes one line:
  `rank N method ntri n_steiner status time_projection time_triangulation`.
  `status` is `ok`, `incomplete` or `error` (with a message).
- Optionally it dumps the triangles in the same `T` format as the EarCut dumps, so
  `figs.py` can draw them.
- It is built with `-O2`, plus `-fopenmp` for the parallel runs.

## 5. Checks of correctness (for every method and every ring)

Test 2 below needs the area that a ring encloses on the unit sphere. On a sphere a
triangle's area is its spherical excess (sum of its angles minus π), so polygon areas are
computed from the same formula.

1. **Count:** a simple ring of N vertices without Steiner points gives N−2 triangles.
2. **Area:** the sum of the triangle areas equals the ring's area, to within 1e-9 relative
   error. A mismatch shows holes or overlaps.
3. **Orientation:** all triangles turn the same way as the ring. Inverted triangles show
   folds, like the inverted remainder in the Honshu failure.
4. **Edges:** every ring edge appears in exactly one triangle, unless mode (b) split it.
5. **Arc vs chord:** for the projection, record the largest gap between an edge's
   great-circle arc and its straight image. This checks that the projection does not
   create overlaps.

These checks run in a small checker. It can be a C++ part of `tri_driver` or a Python
script reusing `ring.py`.

## 6. Test cases

### 6.1 Known bad rings (from earcut.pdf §4 and gshhs-data-notes.md)

| file | ring | problem |
|---|---|---|
| i_L1 | 102218 | Honshu: EarCut fails because of the transposed rotation |
| i_L1 | 194020 | Maine island: crosses itself twice |
| h_L1 | 806054 | same island at high resolution: crosses itself twice, EarCut succeeds |
| i_L2 | 81960 | 4 vertices, two about 1e-6 rad apart |
| i_L2 | 87722 | Ob–Irtysh: thin river, EarCut OpenMP stops early, serial needs 392 passes |
| h_L2 | the 13 failing rings | OpenMP early stop |
| h_L6 | 17477 | Antarctica, passes through the South Pole twice |

For each: status, triangle count, Steiner points, area error, and a figure (one panel per
method, in the gnomonic style of `figs.py`).

### 6.2 Whole layers

Use all of c, i, h for L1, L2, L3 and L5. Add L6 at i and h, since that is where the
problems are. For each file and method, record:
- the number of rings not complete or in error;
- the total of Steiner points;
- the total time.

This gives the failure table of earcut.pdf §4.1 extended with two new columns.

### 6.2b Natural Earth layers

Use `ne_110m_land`, `ne_50m_land`, `ne_10m_land` and the matching `ne_*_lakes`, for every
method, with the same counts as §6.2 plus the number of holes.

- Any ring that fails becomes a test case like those of §6.1, with a figure.
- Compare the time against the GSHHS file of similar size, since the vertex counts differ.

### 6.3 Synthetic cases (optional)

Generated rings for clean comparisons:
- star polygons (many reflex vertices);
- a spiral (worst case for ear cutting);
- a ring with a small loop added (controlled self-crossing).

## 7. Performance measurements

- Per ring: time against N for each method, from 4 vertices up to Eurasia h (139 785).
  Plot it on log–log axes, like `fig_passes_vs_n`. Separate the projection time from the
  triangulation time. Repeat small rings enough times to get a stable time.
- Whole layers: as in Land.cc, the triangulation of all rings, measured in a serial run
  (1 thread) and a parallel run over rings (14 cores, OpenMP loop like
  `Land::layer_t::triangulate`).
  - **Thread safety:** check whether Triangle is thread-safe; it is believed to use global
    state, to be confirmed. If it is not, only its serial times are comparable.
  - CDT objects are independent, so CDT should be safe to run in parallel.
- Memory: peak RSS for the largest layer (`/usr/bin/time -v`).
- Reference times to beat (earcut.pdf): Eurasia h 0.31 s serial, 0.11 s OpenMP; whole
  h_L1 2.9 s serial-ring, 2.2 s with OpenMP.

## 8. Secondary: triangle quality

glgrib fills land in one colour, so thin triangles are not a visible problem. They could
matter later for other uses, though, such as texturing or interpolating along the coast.
Record the smallest angle and a histogram of angles for each method, which costs little.
Do not use refinement (Triangle `-q`), since it adds many vertices.

## 9. Optional: try it inside glgrib

If CDT does well, add a temporary switch in `Land::layer_t::triangulate`, for example the
environment variable `GLGRIB_TRIANGULATOR=cdt`. Then:
- compare offscreen images (`glgrib-egl --render.offscreen.on`) with EarCut's;
- compare the load time of whole layers in the real program.

As with the earlier work, this goes in a patch only, and EarCut.cc/Land.cc are restored
afterwards.

## 10. Deliverables

- `tools/tri_driver.cc`, the checker, and a `tools/make-comparison.sh` script that
  downloads, builds, runs and plots everything.
- Data dumps in the scratchpad or a work directory, not in the repository.
- A short LaTeX report, `share/doc/earcut/triangulation-comparison.pdf`, or a new chapter
  of `earcut.tex`. It contains the failure table, the timing plots, the bad-ring figures
  and a recommendation. The possible recommendations are:
  1. fix EarCut;
  2. replace it with CDT;
  3. keep EarCut and fall back to CDT when it fails;
  4. clean the data offline (see gshhs-data-notes.md) and keep EarCut;
  5. switch glgrib to Natural Earth, if it proves cleaner. This would need hole support in
     the loader.

## 11. Order of work

1. Download and build Triangle and CDT; test the stereographic projection on a few
   crude rings.
2. Write `tri_driver` with the earcut and cdt methods, then add Triangle.
3. Write the checker; validate it on EarCut results already known (Honshu fails, Australia
   passes).
4. Run the bad rings (§6.1) and draw their figures.
5. Run whole layers (§6.2) and the timings (§7).
5b. Download Natural Earth, add hole handling to `tri_driver`, run its data checks and
    layers (§2b, §6.2b).
6. Optionally, the synthetic cases (§6.3) and the glgrib integration (§9).
7. Write the report.

## 12. Open points to decide later

- Whether a CDT result with Steiner points is acceptable in glgrib, which would need a
  vertex buffer change in Land.cc.
- For self-crossing rings, which answer is "right": even–odd filling (CDT), splitting at the
  crossing, or fixing the data first.
- Whether the comparison should also include the EarCut fallback mode (§10, option 3).
- If Natural Earth is adopted: how to provide the GSHHS levels glgrib uses (lakes, islands
  in lakes) from Natural Earth's separate files.
