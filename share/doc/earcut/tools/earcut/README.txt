Copy of glgrib's EarCut (src/glGrib/EarCut.cc, include/glGrib/EarCut.h) as it was when glgrib
switched to the constrained Delaunay triangulation (CDT) and EarCut was removed (2026-11-26).
It is kept unchanged, so that the tools in share/doc/earcut/tools (tri_driver, earcut_driver)
can rebuild the results of earcut.pdf and triangulation-comparison.pdf.
It still includes glGrib/Trigonometry.h from the top-level include directory.
