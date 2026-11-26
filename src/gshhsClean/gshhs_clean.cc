// Offline cleaning of coastline shapefiles (GSHHS, Natural Earth) for glgrib.
//
// Usage : gshhs_clean [options] in.shp [out.shp]
//
//   Without out.shp, only check and report (nothing is written).
//
// Options
//   -eps=RAD      merge consecutive points closer than RAD radians (default 0 : merge only
//                 points that are identical after glgrib's float conversion)
//   -maxdrop=F    a reversed loop is dropped only if its area is below F times the area of the
//                 loop that is kept (default 0.01) ; larger ones are reported, not repaired
//   -q            do not print the per-ring log, only the summary
//
// What is done, ring by ring (one ring = one part of a record), with the coordinates seen by
// glgrib (degrees -> float radians, as glGrib::ShapeLib::read) :
//
//   1. The closing point is dropped (as glgrib does) ; consecutive duplicate points are merged.
//   2. Self-crossings are found with CDT (artem-ogre/CDT, intersecting constraint edges not
//      allowed) in a stereographic projection centred on the ring. At a crossing of edges i and
//      j, the ring is split at the intersection point P into two loops :
//        A = P, i+1 ... j        B = P, j+1 ... i
//      The loop with the larger area is kept. The other one is
//        - dropped if it winds the other way (a twist, e.g. Douglas-Peucker artefacts) and is
//          small (-maxdrop) ;
//        - kept as a separate ring if it winds the same way (figure-eight) ;
//        - reported as unresolved otherwise (the ring is left as it was).
//      This is repeated until no crossing is left.
//   3. Rings with fewer than 3 points or a zero area (< 1e-16 sr, about 0.004 m^2) are dropped.
//   4. Points where a ring touches itself (same point visited twice) are reported, not changed :
//      CDT handles them, Triangle does not.
//   5. The cleaned record is checked : all its rings together with CDT no-crossing (crossings
//      between parts, e.g. holes sharing an edge, are reported), then a full CDT triangulation
//      whose spherical area is compared with the area of the rings.
//
// Records left without any ring are removed, with their row in the .dbf file. Crossings between
// different records (e.g. a lake crossing the coastline) are not checked.
//
// Log lines : rec R part P off O lon LON lat LAT : message
//   R = record number (from 0), P = part number in the input record, O = offset of the first
//   vertex of the part in the file (same numbering as tri_driver / earcut_driver, closing points
//   counted), LON/LAT = location of the change (degrees).

#include <shapefil.h>
#include <omp.h>
#include "CDT/CDT.h"

#include <vector>
#include <string>
#include <map>
#include <set>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <stdexcept>
#include <algorithm>

static const double R2 = 6371.0e3 * 6371.0e3;  // m^2 per steradian
static const double AREA_MIN = 1e-16;          // steradians

struct vec3
{
  double x = 0, y = 0, z = 0;
  vec3 () {}
  vec3 (double a, double b, double c) : x (a), y (b), z (c) {}
  vec3 operator+ (const vec3 & o) const { return vec3 (x + o.x, y + o.y, z + o.z); }
  vec3 operator- (const vec3 & o) const { return vec3 (x - o.x, y - o.y, z - o.z); }
  vec3 operator* (double s) const { return vec3 (x * s, y * s, z * s); }
};

static double dot (const vec3 & a, const vec3 & b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static vec3 cross (const vec3 & a, const vec3 & b)
{
  return vec3 (a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}
static double length (const vec3 & a) { return std::sqrt (dot (a, a)); }
static vec3 normalize (const vec3 & a) { return a * (1.0 / length (a)); }

// A point as stored in the file (degrees) ; new points (intersections) are computed in double
struct pt_t
{
  double lon, lat;
};

// The point as glgrib sees it : float radians (glGrib::ShapeLib::read, Trigonometry.h)
static void glgribLonLat (const pt_t & p, float * lon, float * lat)
{
  const float deg2rad = M_PI / 180.0f;
  *lon = p.lon * deg2rad;
  *lat = p.lat * deg2rad;
}

static vec3 xyzOf (const pt_t & p)
{
  float lon, lat;
  glgribLonLat (p, &lon, &lat);
  double dlon = lon, dlat = lat;
  return vec3 (std::cos (dlon) * std::cos (dlat), std::sin (dlon) * std::cos (dlat), std::sin (dlat));
}

static pt_t ptOf (const vec3 & v)
{
  pt_t p;
  p.lon = std::atan2 (v.y, v.x) * 180.0 / M_PI;
  p.lat = std::asin (std::max (-1.0, std::min (1.0, v.z))) * 180.0 / M_PI;
  return p;
}

// Signed area of a spherical triangle (Van Oosterom & Strackee), with differences for accuracy
static double sphArea (const vec3 & a, const vec3 & b, const vec3 & c)
{
  double num = dot (a, cross (b - a, c - a));
  double den = 1.0 + dot (a, b) + dot (b, c) + dot (c, a);
  return 2.0 * std::atan2 (num, den);
}

static vec3 centre (const std::vector<vec3> & V)
{
  vec3 c;
  for (auto & v : V)
    c = c + v;
  if (length (c) < 1e-9)
    return vec3 (0, 0, -1);
  return normalize (c);
}

// Signed spherical area of a ring (fan from its centre)
static double ringArea (const std::vector<vec3> & V)
{
  int n = V.size ();
  if (n < 3)
    return 0.0;
  vec3 c = centre (V);
  double a = 0;
  for (int j = 0; j < n; j++)
    a += sphArea (c, V[j], V[(j+1)%n]);
  return a;
}

// Stereographic projection seen from -c
struct proj_t
{
  vec3 c, e1, e2;
  explicit proj_t (const vec3 & c0)
  {
    c = normalize (c0);
    vec3 a = std::fabs (c.z) < 0.9 ? vec3 (0, 0, 1) : vec3 (1, 0, 0);
    e1 = normalize (cross (a, c));
    e2 = cross (c, e1);
  }
  CDT::V2d<double> fwd (const vec3 & p) const
  {
    double d = 1.0 + dot (p, c);
    return CDT::V2d<double> (2.0 * dot (p, e1) / d, 2.0 * dot (p, e2) / d);
  }
  vec3 inv (double x, double y) const
  {
    double r2 = x * x + y * y;
    double t = 4.0 / (4.0 + r2);
    return normalize (e1 * (t * x) + e2 * (t * y) + c * (2.0 * t - 1.0));
  }
};

typedef std::vector<pt_t> ring_t;

struct rec_out_t
{
  std::vector<ring_t> rings;
  std::vector<std::string> log;
  std::map<std::string,int> count;
  std::string status;   // result of the final check
};

struct ctx_t
{
  int rec, part, off;
  rec_out_t * out;
  void log (const pt_t & p, const char * fmt, ...) const __attribute__ ((format (printf, 3, 4)))
  {
    char buf[512];
    va_list ap;
    va_start (ap, fmt);
    vsnprintf (buf, sizeof (buf), fmt, ap);
    va_end (ap);
    char head[160];
    snprintf (head, sizeof (head), "rec %d part %d off %d lon %.6f lat %.6f : ", rec, part, off, p.lon, p.lat);
    out->log.push_back (std::string (head) + buf);
  }
};

struct opts_t
{
  double eps = 0.0;
  double maxdrop = 0.01;
};

// 1. Merge consecutive duplicates (cyclically)
static int mergeDuplicates (ring_t * r, const opts_t & o)
{
  int n0 = r->size ();
  ring_t q;
  vec3 last;
  for (auto & p : *r)
    {
      vec3 v = xyzOf (p);
      if (! q.empty ())
        {
          float lon1, lat1, lon2, lat2;
          glgribLonLat (q.back (), &lon1, &lat1);
          glgribLonLat (p, &lon2, &lat2);
          bool same = (lon1 == lon2) && (lat1 == lat2);
          same = same || ((o.eps > 0) && (length (v - last) < o.eps));
          if (same)
            continue;
        }
      q.push_back (p);
      last = v;
    }
  // Last against first
  while (q.size () > 1)
    {
      float lon1, lat1, lon2, lat2;
      glgribLonLat (q.back (), &lon1, &lat1);
      glgribLonLat (q.front (), &lon2, &lat2);
      bool same = (lon1 == lon2) && (lat1 == lat2);
      same = same || ((o.eps > 0) && (length (xyzOf (q.back ()) - xyzOf (q.front ())) < o.eps));
      if (! same)
        break;
      q.pop_back ();
    }
  *r = q;
  return n0 - (int)q.size ();
}

struct crossing_t
{
  enum { NONE, CROSS, OTHER } kind = NONE;
  int i = -1, j = -1;   // edges i -> i+1 and j -> j+1, i < j
  vec3 P;               // intersection
  bool parallel = false;
  std::string msg;
  std::vector<int> pinch;  // vertices visited twice
};

// 2. Find one crossing with CDT (intersecting constraint edges not allowed)
static crossing_t findCrossing (const ring_t & r)
{
  crossing_t x;
  int n = r.size ();
  std::vector<vec3> V (n);
  for (int k = 0; k < n; k++)
    V[k] = xyzOf (r[k]);
  proj_t pr (centre (V));
  std::vector<CDT::V2d<double> > P (n);
  for (int k = 0; k < n; k++)
    P[k] = pr.fwd (V[k]);

  std::vector<CDT::V2d<double> > verts = P;
  std::vector<CDT::Edge> edges;
  for (int k = 0; k < n; k++)
    edges.push_back (CDT::Edge (k, (k + 1) % n));
  CDT::DuplicatesInfo dup = CDT::RemoveDuplicatesAndRemapEdges (verts, edges);
  for (auto & d : dup.duplicates)
    x.pinch.push_back (d);

  // Unique edge -> ring edge
  std::map<std::pair<int,int>,int> e2k;
  std::vector<CDT::Edge> edges2;
  for (int k = 0; k < n; k++)
    {
      int a = edges[k].v1 (), b = edges[k].v2 ();
      if (a == b)
        continue;
      e2k[std::make_pair (std::min (a, b), std::max (a, b))] = k;
      edges2.push_back (edges[k]);
    }

  CDT::Triangulation<double> cdt (CDT::VertexInsertionOrder::Auto,
                                  CDT::IntersectingConstraintEdges::NotAllowed, 0.0);
  try
    {
      cdt.insertVertices (verts);
      cdt.insertEdges (edges2);
    }
  catch (const CDT::IntersectingConstraintsError & e)
    {
      auto look = [&] (const CDT::Edge & ed)
      {
        int a = ed.v1 (), b = ed.v2 ();
        auto it = e2k.find (std::make_pair (std::min (a, b), std::max (a, b)));
        return it == e2k.end () ? -1 : it->second;
      };
      int i = look (e.e1 ()), j = look (e.e2 ());
      if ((i < 0) || (j < 0))
        {
          x.kind = crossing_t::OTHER;
          x.msg = e.what ();
          return x;
        }
      if (i > j)
        std::swap (i, j);
      x.kind = crossing_t::CROSS;
      x.i = i;
      x.j = j;
      // Intersection of the two segments in the projection plane
      const CDT::V2d<double> & a0 = P[i], & a1 = P[(i+1)%n], & b0 = P[j], & b1 = P[(j+1)%n];
      double dax = a1.x - a0.x, day = a1.y - a0.y, dbx = b1.x - b0.x, dby = b1.y - b0.y;
      double d = dax * dby - day * dbx;
      double la = std::hypot (dax, day), lb = std::hypot (dbx, dby);
      if (std::fabs (d) <= 1e-12 * la * lb)
        {
          x.parallel = true;
          x.P = V[(i+1)%n];
        }
      else
        {
          double t = ((b0.x - a0.x) * dby - (b0.y - a0.y) * dbx) / d;
          t = std::max (0.0, std::min (1.0, t));
          x.P = pr.inv (a0.x + t * dax, a0.y + t * day);
        }
      return x;
    }
  catch (const std::exception & e)
    {
      x.kind = crossing_t::OTHER;
      x.msg = e.what ();
      return x;
    }
  return x;
}

static std::vector<vec3> xyzRing (const ring_t & r)
{
  std::vector<vec3> V (r.size ());
  for (size_t k = 0; k < r.size (); k++)
    V[k] = xyzOf (r[k]);
  return V;
}

// Is loop S (S[0] = the crossing point, shared with K) inside loop K ? Majority vote of the other
// vertices of S, point in polygon in a stereographic projection centred on K
static bool loopInside (const ring_t & S, const ring_t & K)
{
  std::vector<vec3> VK = xyzRing (K);
  proj_t pr (centre (VK));
  int n = K.size ();
  std::vector<CDT::V2d<double> > PK (n);
  for (int k = 0; k < n; k++)
    PK[k] = pr.fwd (VK[k]);
  int in = 0, tot = 0;
  for (size_t s = 1; s < S.size (); s++)
    {
      CDT::V2d<double> p = pr.fwd (xyzOf (S[s]));
      bool c = false;
      for (int i = 0, j = n - 1; i < n; j = i++)
        if (((PK[i].y > p.y) != (PK[j].y > p.y))
         && (p.x < (PK[j].x - PK[i].x) * (p.y - PK[i].y) / (PK[j].y - PK[i].y) + PK[i].x))
          c = ! c;
      in += c;
      tot++;
    }
  return 2 * in > tot;
}

// Clean one ring ; may produce several rings (figure-eights are split)
static void cleanRing (ring_t r, const opts_t & o, const ctx_t & cx, std::vector<ring_t> * result)
{
  std::vector<ring_t> todo;
  todo.push_back (r);
  int iter = 0;

  while (! todo.empty ())
    {
      ring_t q = todo.back ();
      todo.pop_back ();

      int nm = mergeDuplicates (&q, o);
      if (nm > 0)
        {
          cx.log (q.empty () ? r[0] : q[0], "merged %d duplicate point(s)", nm);
          cx.out->count["merged-points"] += nm;
        }

      double area = ringArea (xyzRing (q));
      if ((q.size () < 3) || (std::fabs (area) < AREA_MIN))
        {
          cx.log (q.empty () ? r[0] : q[0], "dropped degenerate ring (%zu points, area %.3g m2)",
                  q.size (), std::fabs (area) * R2);
          cx.out->count["dropped-degenerate"]++;
          continue;
        }

      crossing_t x = findCrossing (q);

      if (x.kind == crossing_t::NONE)
        {
          for (int k : x.pinch)
            {
              cx.log (q[k], "ring touches itself at vertex %d (left as is)", k);
              cx.out->count["pinch"]++;
            }
          result->push_back (q);
          continue;
        }

      if ((x.kind == crossing_t::OTHER) || (++iter > 1000))
        {
          cx.log (q[0], "unresolved : %s", x.kind == crossing_t::OTHER ? x.msg.c_str () : "too many crossings");
          cx.out->count["unresolved"]++;
          result->push_back (q);
          continue;
        }

      int n = q.size (), i = x.i, j = x.j;
      pt_t P = ptOf (x.P);

      // Adjacent edges folded back on each other (a spike) : remove the shared vertex
      if (x.parallel)
        {
          int s = (j == i + 1) ? j : (((i == 0) && (j == n - 1)) ? 0 : -1);
          if (s < 0)
            {
              cx.log (q[i], "unresolved : overlapping edges %d and %d", i, j);
              cx.out->count["unresolved"]++;
              result->push_back (q);
              continue;
            }
          cx.log (q[s], "removed spike vertex %d", s);
          cx.out->count["spike"]++;
          q.erase (q.begin () + s);
          todo.push_back (q);
          continue;
        }

      ring_t A, B;
      A.push_back (P);
      for (int k = i + 1; k <= j; k++)
        A.push_back (q[k]);
      B.push_back (P);
      for (int k = j + 1; k < n + i + 1; k++)
        B.push_back (q[k % n]);

      double aA = ringArea (xyzRing (A)), aB = ringArea (xyzRing (B));
      bool keepA = std::fabs (aA) >= std::fabs (aB);
      ring_t & K = keepA ? A : B, & S = keepA ? B : A;
      double aK = keepA ? aA : aB, aS = keepA ? aB : aA;
      int nS = S.size () - 1;  // without P

      // Nonzero winding rule : land is where the ring winds around at least once
      bool reversed = aK * aS < 0, inside = loopInside (S, K);
      bool pole = std::fabs (P.lat) > 89.999;  // float images of the pole points (Antarctica)
      const char * where = pole ? " (pole points)" : "";
      if (pole)
        cx.out->count["pole"]++;

      if (! inside)
        {
          // Separate lobe (figure-eight, or bowtie twist of a thin peninsula by Douglas-Peucker) :
          // a ring of its own, with the orientation of K
          if (reversed)
            std::reverse (S.begin () + 1, S.end ());
          cx.log (P, "crossing of edges %d and %d : split off %sloop as a new ring (%d points, %.3g m2)%s",
                  i, j, reversed ? "reversed " : "", nS, std::fabs (aS) * R2, where);
          cx.out->count[reversed ? "split-reversed" : "split"]++;
          todo.push_back (K);
          todo.push_back (S);
        }
      else if (! reversed)
        {
          // Folded over K : winding number 2, already covered by K
          cx.log (P, "crossing of edges %d and %d : dropped loop folded inside the ring (%d points, %.3g m2)%s",
                  i, j, nS, std::fabs (aS) * R2, where);
          cx.out->count["dropped-fold"]++;
          todo.push_back (K);
        }
      else if (std::fabs (aS) <= o.maxdrop * std::fabs (aK))
        {
          // Winding number 0 inside K : a hole, too small to keep
          cx.log (P, "crossing of edges %d and %d : filled reversed loop inside the ring (%d points, %.3g m2)%s",
                  i, j, nS, std::fabs (aS) * R2, where);
          cx.out->count["filled-hole"]++;
          todo.push_back (K);
        }
      else
        {
          cx.log (P, "unresolved : crossing of edges %d and %d, reversed loop inside the ring too large "
                  "(%d points, %.3g m2, %.2g of the ring)", i, j, nS, std::fabs (aS) * R2,
                  std::fabs (aS) / std::fabs (aK));
          cx.out->count["unresolved"]++;
          result->push_back (q);
        }
    }
}

// 5. Final check of a polygon (outer ring and its holes) : crossings between rings, then full
// triangulation and areas
static std::string checkPolygon (const std::vector<ring_t> & rings, const ctx_t & cx)
{

  std::vector<vec3> V;
  std::vector<int> rb, rn;
  for (auto & r : rings)
    {
      rb.push_back (V.size ());
      rn.push_back (r.size ());
      for (auto & p : r)
        V.push_back (xyzOf (p));
    }
  int nV = V.size ();

  // Rings are not nested deeper than outer/holes : signed sum, holes wind the other way
  double aring = 0;
  for (size_t k = 0; k < rings.size (); k++)
    aring += ringArea (std::vector<vec3> (V.begin () + rb[k], V.begin () + rb[k] + rn[k]));

  proj_t pr (centre (std::vector<vec3> (V.begin (), V.begin () + rn[0])));
  std::vector<CDT::V2d<double> > verts (nV);
  for (int k = 0; k < nV; k++)
    verts[k] = pr.fwd (V[k]);
  std::vector<CDT::Edge> edges;
  for (size_t r = 0; r < rings.size (); r++)
    for (int k = 0; k < rn[r]; k++)
      edges.push_back (CDT::Edge (rb[r] + k, rb[r] + (k + 1) % rn[r]));
  CDT::DuplicatesInfo dup = CDT::RemoveDuplicatesAndRemapEdges (verts, edges);
  std::vector<CDT::Edge> edges2;
  for (auto & e : edges)
    if (e.v1 () != e.v2 ())
      edges2.push_back (e);

  std::string status;

  // Edges used twice (e.g. two holes sharing an edge) : not a crossing for CDT, but the even-odd
  // rule then removes the edge, and the area is wrong
  {
    std::set<std::pair<int,int> > seen;
    for (auto & e : edges2)
      {
        auto key = std::make_pair (std::min (e.v1 (), e.v2 ()), std::max (e.v1 (), e.v2 ()));
        if (seen.insert (key).second)
          continue;
        int k0 = 0;
        for (int k = 0; k < nV; k++)
          if ((int)dup.mapping[k] == (int)key.first)
            {
              k0 = k;
              break;
            }
        cx.log (ptOf (V[k0]), "an edge is used twice (rings share an edge, left as is)");
        cx.out->count["shared-edge"]++;
        status = "rings-cross ";
        break;
      }
  }

  if ((rings.size () > 1) && status.empty ())
    {
      CDT::Triangulation<double> cdt (CDT::VertexInsertionOrder::Auto,
                                      CDT::IntersectingConstraintEdges::NotAllowed, 0.0);
      try
        {
          cdt.insertVertices (verts);
          cdt.insertEdges (edges2);
        }
      catch (const CDT::IntersectingConstraintsError & e)
        {
          // Back to an original vertex for the location
          int u = e.e1 ().v1 (), k0 = 0;
          for (int k = 0; k < nV; k++)
            if ((int)dup.mapping[k] == u)
              {
                k0 = k;
                break;
              }
          cx.log (ptOf (V[k0]), "rings of the polygon cross each other (left as is)");
          cx.out->count["rings-cross"]++;
          status = "rings-cross ";
        }
      catch (const std::exception & e)
        {
        }
    }

  CDT::Triangulation<double> cdt (CDT::VertexInsertionOrder::Auto,
                                  CDT::IntersectingConstraintEdges::TryResolve, 0.0);
  try
    {
      cdt.insertVertices (verts);
      cdt.insertEdges (edges2);
      cdt.eraseOuterTrianglesAndHoles ();
    }
  catch (const std::exception & e)
    {
      cx.log (ptOf (V[0]), "final check : CDT error (%s)", e.what ());
      return status + "error";
    }

  int nu = verts.size ();
  std::vector<vec3> W (cdt.vertices.size ());
  std::vector<int> u2l (nu, -1);
  for (int k = nV - 1; k >= 0; k--)
    u2l[dup.mapping[k]] = k;
  for (size_t k = 0; k < cdt.vertices.size (); k++)
    W[k] = (int)k < nu ? V[u2l[k]] : pr.inv (cdt.vertices[k].x, cdt.vertices[k].y);

  double s = 0, sabs = 0;
  std::vector<double> at;
  for (auto & t : cdt.triangles)
    {
      double a = sphArea (W[t.vertices[0]], W[t.vertices[1]], W[t.vertices[2]]);
      at.push_back (a);
      s += a;
      sabs += std::fabs (a);
    }
  int inv = 0;
  for (double a : at)
    if (a * (s >= 0 ? 1 : -1) < -1e-15)
      inv++;
  double err = std::fabs (std::fabs (s) - std::fabs (aring)) / std::fabs (aring);
  int nsteiner = cdt.vertices.size () - nu;

  if ((err > 1e-6) || (inv > 0))
    {
      cx.log (ptOf (V[0]), "final check : bad triangulation (area error %.2g, %d inverted)", err, inv);
      return status + "bad";
    }
  if (nsteiner > 0)
    {
      cx.log (ptOf (V[0]), "final check : %d point(s) added by CDT (edges crossing after projection)", nsteiner);
      return status + "ok-steiner";
    }
  return status + "ok";
}

// Planar signed area in lon/lat (shapefile convention : outer rings clockwise, holes counter-clockwise)
static double llArea (const ring_t & r)
{
  double s = 0;
  for (size_t k = 0, n = r.size (); k < n; k++)
    s += r[k].lon * r[(k+1)%n].lat - r[(k+1)%n].lon * r[k].lat;
  return 0.5 * s;
}

static bool pipLonLat (const pt_t & p, const ring_t & r)
{
  bool in = false;
  for (size_t i = 0, j = r.size () - 1; i < r.size (); j = i++)
    if (((r[i].lat > p.lat) != (r[j].lat > p.lat))
     && (p.lon < (r[j].lon - r[i].lon) * (p.lat - r[i].lat) / (r[j].lat - r[i].lat) + r[i].lon))
      in = ! in;
  return in;
}

// Final check of a record : the rings are grouped into polygons (outer ring + holes, as in
// tri_driver), and each polygon is checked on its own. Polygons of the same record are not
// checked against each other.
static std::string checkRecord (const std::vector<ring_t> & rings, const ctx_t & cx)
{
  if (rings.empty ())
    return "empty";

  std::vector<std::vector<ring_t> > polys;
  std::vector<int> holes;
  for (size_t k = 0; k < rings.size (); k++)
    if ((rings.size () > 1) && (llArea (rings[k]) > 0))
      holes.push_back (k);
    else
      polys.push_back (std::vector<ring_t> (1, rings[k]));
  for (int h : holes)
    {
      size_t q = 0;
      for (size_t p = 0; p < polys.size (); p++)
        if (pipLonLat (rings[h][0], polys[p][0]))
          {
            q = p;
            break;
          }
      if (polys.empty ())
        polys.push_back (std::vector<ring_t> ());
      polys[q].push_back (rings[h]);
    }

  // Worst status first
  static const char * order[] = {"error", "bad", "rings-cross ok-steiner", "rings-cross ok", "ok-steiner", "ok"};
  int worst = 5;
  for (auto & p : polys)
    {
      std::string s = checkPolygon (p, cx);
      for (int k = 0; k < 6; k++)
        if ((s == order[k]) || ((k < 2) && (s.size () > strlen (order[k]))
                                && (s.substr (s.size () - strlen (order[k])) == order[k])))
          {
            worst = std::min (worst, k);
            break;
          }
    }
  return order[worst];
}

static rec_out_t processRecord (SHPObject * shape, int rec, int off, const opts_t & o)
{
  rec_out_t out;
  int nparts = shape->nParts > 0 ? shape->nParts : 1;
  for (int p = 0; p < nparts; p++)
    {
      int j1 = shape->nParts > 0 ? shape->panPartStart[p] : 0;
      int j2 = (p + 1 < shape->nParts) ? shape->panPartStart[p+1] : shape->nVertices;
      ctx_t cx {rec, p, off + j1, &out};
      ring_t r;
      for (int j = j1; j < j2; j++)
        r.push_back (pt_t {shape->padfX[j], shape->padfY[j]});
      if (r.empty ())
        continue;
      // Closing point
      if ((r.size () > 1) && (r.front ().lon == r.back ().lon) && (r.front ().lat == r.back ().lat))
        r.pop_back ();
      else
        {
          cx.log (r[0], "ring was not closed");
          out.count["not-closed"]++;
        }
      cleanRing (r, o, cx, &out.rings);
    }
  ctx_t cx {rec, -1, off, &out};
  out.status = checkRecord (out.rings, cx);
  return out;
}

int main (int argc, char * argv[])
{
  opts_t o;
  bool quiet = false;
  std::vector<std::string> files;
  for (int i = 1; i < argc; i++)
    {
      std::string a = argv[i];
      if (a.substr (0, 5) == "-eps=") o.eps = std::atof (a.c_str () + 5);
      else if (a.substr (0, 9) == "-maxdrop=") o.maxdrop = std::atof (a.c_str () + 9);
      else if (a == "-q") quiet = true;
      else if (a[0] == '-')
        {
          fprintf (stderr, "Unknown option %s\n", a.c_str ());
          return 1;
        }
      else files.push_back (a);
    }
  if ((files.size () < 1) || (files.size () > 2))
    {
      fprintf (stderr, "Usage: %s [-eps=RAD] [-maxdrop=F] [-q] in.shp [out.shp]\n", argv[0]);
      return 1;
    }

  auto base = [] (const std::string & f)
  {
    return (f.size () > 4) && (f.substr (f.size () - 4) == ".shp") ? f.substr (0, f.size () - 4) : f;
  };
  std::string in = base (files[0]);

  SHPHandle fi = SHPOpen (in.c_str (), "rb");
  if (fi == nullptr)
    {
      fprintf (stderr, "Cannot open %s.shp\n", in.c_str ());
      return 1;
    }
  int nent, ntype;
  double minb[4], maxb[4];
  SHPGetInfo (fi, &nent, &ntype, minb, maxb);
  if ((ntype != SHPT_POLYGON) && (ntype != SHPT_POLYGONZ) && (ntype != SHPT_POLYGONM))
    {
      fprintf (stderr, "%s.shp : not a polygon shapefile (type %d)\n", in.c_str (), ntype);
      return 1;
    }

  std::vector<SHPObject *> shapes (nent);
  std::vector<int> offs (nent);
  int off = 0;
  for (int i = 0; i < nent; i++)
    {
      shapes[i] = SHPReadObject (fi, i);
      offs[i] = off;
      off += shapes[i] ? shapes[i]->nVertices : 0;
    }
  SHPClose (fi);

  std::vector<rec_out_t> outs (nent);
  double t0 = omp_get_wtime ();
#pragma omp parallel for schedule (dynamic, 16)
  for (int i = 0; i < nent; i++)
    if (shapes[i] != nullptr)
      outs[i] = processRecord (shapes[i], i, offs[i], o);
  double t1 = omp_get_wtime ();

  std::map<std::string,int> count, status;
  int nrings = 0;
  for (int i = 0; i < nent; i++)
    {
      if (! quiet)
        for (auto & l : outs[i].log)
          printf ("%s\n", l.c_str ());
      for (auto & kv : outs[i].count)
        count[kv.first] += kv.second;
      status[outs[i].status]++;
      nrings += outs[i].rings.size ();
    }

  int nwritten = 0;
  if (files.size () == 2)
    {
      std::string outb = base (files[1]);
      if (outb == in)
        {
          fprintf (stderr, "Output must differ from input\n");
          return 1;
        }
      SHPHandle fo = SHPCreate (outb.c_str (), SHPT_POLYGON);
      DBFHandle di = DBFOpen (in.c_str (), "rb");
      DBFHandle dout = di ? DBFCloneEmpty (di, outb.c_str ()) : nullptr;
      if ((fo == nullptr) || (di && (dout == nullptr)))
        {
          fprintf (stderr, "Cannot create %s\n", outb.c_str ());
          return 1;
        }
      for (int i = 0; i < nent; i++)
        {
          const auto & rings = outs[i].rings;
          if (rings.empty ())
            continue;
          std::vector<int> start;
          std::vector<double> X, Y;
          for (auto & r : rings)
            {
              start.push_back (X.size ());
              for (auto & p : r)
                {
                  X.push_back (p.lon);
                  Y.push_back (p.lat);
                }
              X.push_back (r[0].lon);  // closing point
              Y.push_back (r[0].lat);
            }
          SHPObject * obj = SHPCreateObject (SHPT_POLYGON, -1, start.size (), start.data (), nullptr,
                                             X.size (), X.data (), Y.data (), nullptr, nullptr);
          SHPWriteObject (fo, -1, obj);
          SHPDestroyObject (obj);
          if (dout)
            DBFWriteTuple (dout, nwritten, (void *) DBFReadTuple (di, i));
          nwritten++;
        }
      SHPClose (fo);
      if (dout)
        DBFClose (dout);
      if (di)
        DBFClose (di);
      // Projection file, if any
      FILE * pi = fopen ((in + ".prj").c_str (), "rb");
      if (pi)
        {
          FILE * po = fopen ((outb + ".prj").c_str (), "wb");
          char buf[4096];
          size_t n;
          while (po && ((n = fread (buf, 1, sizeof (buf), pi)) > 0))
            fwrite (buf, 1, n, po);
          if (po)
            fclose (po);
          fclose (pi);
        }
    }

  printf ("# file %s.shp records %d rings_out %d time %.2f", in.c_str (), nent, nrings, t1 - t0);
  for (auto & kv : count)
    printf (" %s=%d", kv.first.c_str (), kv.second);
  printf (" | check");
  for (auto & kv : status)
    printf (" %s=%d", kv.first.c_str (), kv.second);
  if (files.size () == 2)
    printf (" | written %d records", nwritten);
  printf ("\n");

  for (auto s : shapes)
    if (s)
      SHPDestroyObject (s);
  return 0;
}
