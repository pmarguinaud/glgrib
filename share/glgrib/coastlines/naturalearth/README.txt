Natural Earth 10m land
======================

ne_10m_land.{shp,shx,dbf,prj,cpg}: land polygons at 1:10 million scale, from Natural Earth
version 5.1.1 (see ne_10m_land.VERSION.txt), https://www.naturalearthdata.com/
Format: ESRI shapefile, polygons, WGS84 longitude/latitude in degrees, 11 records
(6 838 rings).

Licence: public domain. Natural Earth data may be used for any purpose without
permission; crediting "Made with Natural Earth" is appreciated.
See https://www.naturalearthdata.com/about/terms-of-use/

Changes from the original (2026-11-26, src/gshhsClean/gshhs_clean)
-------------------------------------------------------------------
Only the Antarctica ring (record 0) changed:
  - the points next to the South Pole make two reversed loops of 362 points and about
    0.1 m2 once converted to single precision, as glgrib does: filled;
  - a fold of 3.4e4 m2 at the 180 degree cut (180W 84.4S): dropped.
After cleaning, the file passes "gshhs_clean ne_10m_land.shp" (check only) with no
crossing left. Without these repairs EarCut, which glgrib used before 2026-11-26, aborts
on this file; CDT draws both versions.

ne_10m_land.db is the SQLite copy of the .dbf that glgrib makes on first use.

Use
---
  glgrib --land.on --land.layers[0].path coastlines/naturalearth/ne_10m_land.shp \
         --land.layers[1].off --land.layers[2].off --land.layers[3].off
