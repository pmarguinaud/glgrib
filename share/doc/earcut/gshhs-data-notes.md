# GSHHS data quality and cleaning — notes (2026-11-25)

Follow-up to `earcut.pdf` §4.3 (invalid input rings). The cleaning tool is written (see the end).

## GSHHS version

- Local copy: **GSHHG 2.3.7 (15 June 2017)**, see `share/glgrib/coastlines/gshhg/README.TXT`.
- This is still the latest release: https://www.soest.hawaii.edu/wessel/gshhg/
- NOAA/NCEI retired its copy in May 2025 with no further updates, so 2.3.7 is very likely final.
- Licence: LGPL, so a corrected copy may be distributed if the changes are documented.

## File naming: `GSHHS_<resolution>_L<level>.shp`

| letter | resolution | in `share/glgrib/coastlines/shp/` |
|---|---|---|
| f | full (original data, not simplified) | no |
| h | high | yes |
| i | intermediate | yes |
| l | low | only L2–L6 |
| c | crude (glgrib default) | yes |

Each resolution is made from the one above with Douglas–Peucker simplification,
which removes about 80 % of the points each time (`gshhg/SHAPEFILES.TXT`).

| level | contents |
|---|---|
| L1 | continents and ocean islands, except Antarctica |
| L2 | lakes |
| L3 | islands in lakes |
| L4 | ponds on islands in lakes |
| L5 | Antarctica, ice-front version |
| L6 | Antarctica, grounding-line version |

## Are invalid rings expected?

- GSHHS promises polygons that do not cross themselves or each other; that is what
  "self-consistent" means (Wessel & Smith 1996). So a self-crossing ring is a data error.
- Such errors are known to slip through. The README history records crossings being fixed
  (2.3.2, 2.3.6) and thanks a tester for reporting "erratic data points and crossings".
- Douglas–Peucker simplification does not check that the result is still a simple polygon,
  so the lower resolutions can gain new crossings.

### Example: the Maine island (lon −69.78…−69.71, lat 43.75…43.88)

| file | ring (first vertex offset) | vertices | self-crossings |
|---|---|---|---|
| h_L1 | 806054 | 112 | 2 (segments 24×26, 104×106) |
| i_L1 | 194020 | 20 | 2 (segments 1×18, 14×16) |

The crossings are at different places in the two versions, so the `h` version is already
wrong. The full-resolution (`f`) files are not available locally, so it is unknown whether
the original data is wrong too. `EarCut` still triangulated the `h` ring without failure
(no failures in `h_L1`).

### Other invalid rings found (earcut.pdf §4.3)

- i_L1 194020: self-crossing (above).
- i_L2 81960: 4 vertices, two of them about 1e-6 rad apart.
- h_L6 17477: Antarctica, passes through the South Pole twice. It may do so on purpose,
  to close the polygon over the pole; this has not been checked.

## Decision so far: clean the data offline, keep a safety net in glgrib

Why clean in advance with a separate tool:

- **The data no longer changes.** It is frozen since 2017 and glgrib ships its own copy,
  so cleaning once is enough.
- **Checking at every load would be wasted work.** A simple self-crossing test compares every
  pair of edges, so its cost grows with the square of the ring size; a grid or sweep-line
  test is faster but still extra code and load time.
- **Repairs need choices that do not belong in a viewer** (split the ring at the crossing,
  or drop the small loop). An offline tool decides once and logs its changes.

What to keep in glgrib anyway:

- In `EarCut`, when a pass cuts nothing, stop cleanly. Print a warning if triangles are
  missing and return what has been cut. This costs nothing and protects against data the
  tool never saw.
- Fix the two code bugs in any case, since they are independent of the data:
  - the transposed rotation (`R` should be `transpose(R)` in `processRing`);
  - the OpenMP early stop.

## Cleaning tool (written 2026-11-26)

`src/gshhsClean/gshhs_clean.cc`, built by `make` in `src/gshhsClean` (not part of the default
build; needs shapelib and the CDT headers vendored in `include/CDT`).
`src/gshhsClean/clean-gshhs.sh OUTDIR` builds it if needed and cleans every `GSHHS_*.shp` shipped
with glgrib into OUTDIR, with one `.log` per file. The script itself does not touch the shipped files.

```
gshhs_clean [-eps=RAD] [-maxdrop=F] [-q] in.shp [out.shp]     # without out.shp : check only
```

Per ring, with the coordinates glgrib sees (float radians):

1. Drop the closing point; merge consecutive points that are identical in float (or closer
   than `-eps`).
2. Find a self-crossing with CDT, crossing edges not allowed, in a stereographic projection
   centred on the ring. Split the ring at the intersection point into two loops, and apply
   the **nonzero winding rule** (land is where the ring winds at least once):
   - smaller loop outside the other: a separate lobe → kept as a new ring, re-oriented if it
     was reversed (bowtie twists of thin peninsulas keep their land);
   - inside, same orientation: a fold covering land twice → dropped;
   - inside, reversed: a hole → filled if smaller than `-maxdrop` (0.01) of the ring, else
     reported and left as is.
   Repeat until CDT finds no crossing.
3. Drop rings with fewer than 3 points or an area below 1e-16 sr (0.004 m²).
4. Report points where a ring touches itself; they are left as they are.
5. Final check per polygon (outer ring + holes): edges used twice, crossings between rings,
   then a full CDT triangulation compared with the spherical area of the rings.

Records left empty are removed with their `.dbf` row; split lobes become extra parts of the same
record, which glgrib reads as separate rings. Crossings between different records are not
checked.

### Results on the GSHHS files (all 22, 2.5 s in total)

All files pass the final check, and `tri_driver ... cdt-noX` finds no invalid ring in the output
(it found 1–2 per file before). EarCut's failures are unchanged (h_L1: 38 before and after):
they come from its float arithmetic, not from the data.

| case | files | change |
|---|---|---|
| Antarctica pole points: (180,−90) and (0,−90) become two points 1e-7 rad apart in float, making a loop of a few mm² | all L5, L6 | loop dropped |
| Antarctica, start of the ring near 162.5°E 76°S | L5 c/i/l | fold of about 2e4 m² dropped |
| Drygalski Ice Tongue (162.9°E 75.4°S): the two sides of a sea inlet cross | i_L5 | reversed loop of 59 km² filled (the tip of the inlet becomes land) |
| same place | l_L5 | reversed loop of 13 km² filled |
| Antarctica grounding line, small twists | h_L6 | 2 lobes (3.6e4 and 818 m²) kept as separate rings |
| Maine island, i_L1 194020 | i_L1 | fold of 3.1e5 m² dropped; lobe of 17 m² kept as a separate ring |
| Maine island, h_L1 806054 | h_L1 | 2 folds (5.6e3 and 7.0e3 m²) dropped |
| near-duplicate lake ring, i_L2 81960 | i_L2 | bowtie: lobe of 43 m² kept as a separate ring |

Natural Earth (5.1.1 and Magics' 3.0.1) also goes through. Repeated points are merged
(1,164 in 50m lakes). Its antimeridian slit and rows of points at −90 give a few folds and
filled loops of up to 4.5e4 m², and 3.0.1 loses 348 zero-area rings (151 records). The two
holes sharing an edge in the 3.0.1 Americas are reported (`shared-edge`), not repaired.

The glgrib binary loads a cleaned file, including its `.dbf`, and renders it.

### Open points

- Installed on 2026-11-26: the `.shp`/`.shx` of the 11 changed files replaced the shipped ones in
  `share/glgrib/coastlines/shp` (the `.dbf` differed only by the header date and were kept, so the
  `.db` files stay valid). The changes are listed in `share/glgrib/coastlines/shp/README.cleaning.txt`
  (GSHHS is LGPL, so they must be documented). Natural Earth 10m land (cleaned) is in
  `share/glgrib/coastlines/naturalearth`, with its own `README.txt`.
- The filled Drygalski inlet (59 km² at i) is a choice; `-maxdrop=1e-6` would leave it as is and
  report it instead.
- Tiny split lobes (17 m², 43 m²) could be dropped with a minimum area option.
- Optionally, download the full-resolution GSHHG shapefiles from SOEST to check whether the
  original data already has the Maine crossing.
