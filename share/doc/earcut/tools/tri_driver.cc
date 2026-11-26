// Compare polygon triangulation methods on coastline shapefiles (GSHHS, Natural Earth).
//
// Usage : tri_driver file.shp METHOD [options] [rank ...|all|min=N]
//
// METHOD
//   earcut       glGrib::EarCut::processRing as in glgrib (OpenMP inside rings of >= 300 points)
//   earcut-fix   same code with transpose(R) and serial passes (built from a sed'ed copy of EarCut.cc)
//   triangle     Shewchuk's Triangle, switches -pzQBP (constrained Delaunay)
//   triangle-Y   same with -Y (no Steiner points on segments, except at segment intersections)
//   cdt          artem-ogre/CDT, TryResolve intersections, eraseOuterTrianglesAndHoles (even-odd)
//   cdt-noX      CDT, intersecting constraint edges not allowed (error), eraseOuterTrianglesAndHoles
//   cdt-outer    CDT, TryResolve, eraseOuterTriangles (keeps holes filled; for rings without holes)
//
// Options
//   par          triangulate the polygons in an OpenMP parallel loop (like Land::layer_t::triangulate)
//   nocheck      do not run the correctness checks (for timings)
//   dump=dir     write dir/poly_<rank>.txt (vertices and triangles) for each selected polygon
//   noholes      ignore holes (inner rings) for all methods
//
// A polygon is an outer ring and its holes; holes are the counter-clockwise parts of a record
// (shapefile convention) ; GSHHS has no holes. Rings are read like Land.cc does : the last point
// of each part (equal to the first one) is dropped. Coordinates are converted to float, like glgrib.
// Rank = offset of the first vertex of the outer ring in the file (same numbering as earcut_driver).
//
// Output : one line per polygon, then a summary line starting with '#'.
//   rank N nholes ntri expected nsteiner status area_err overlap inverted missing antipode t_proj t_tri
//   max_sagitta area inv_area
// (areas in steradians ; multiply by 6371^2 for km^2)

#include "glGrib/EarCut.h"

#include <shapefil.h>
#include <omp.h>

#include <CDT.h>

extern "C"
{
#define REAL double
#define VOID void
#define ANSI_DECLARATORS
#include "triangle.h"
#undef REAL
#undef VOID
}

#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <set>
#include <map>
#include <tuple>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <csetjmp>
#include <stdexcept>
#include <algorithm>

// From earcut_fix_wrap.cc
void earcutFix (const std::vector<float> &, int, int, int, int *, std::vector<unsigned int> *, bool);

// Triangle error handler (triexit is redirected here in the patched copy of triangle.c)
static thread_local jmp_buf tri_jmp;
extern "C" void triangle_exit_hook (int status)
{
  longjmp (tri_jmp, status == 0 ? 1 : status);
}

typedef glm::dvec3 vec3;

struct ring_t
{
  int off;   // offset of first vertex in lonlat
  int len;   // number of vertices (closing point dropped)
  double sarea_ll; // planar signed area in lon/lat (shapefile orientation)
};

struct poly_t
{
  int rank;
  ring_t outer;
  std::vector<ring_t> holes;
};

struct result_t
{
  int N = 0, nholes = 0, ntri = 0, expected = 0, nsteiner = 0;
  std::string status;
  double area_err = 0, overlap = 0, area = 0, inv_area = 0;
  int inverted = 0, missing = 0, antipode = 0;
  double t_proj = 0, t_tri = 0;
  double max_sagitta = 0;
};

static std::vector<float> lonlat;  // radians, like glgrib

static vec3 xyzOf (int k)
{
  double lon = lonlat[2*k+0], lat = lonlat[2*k+1];
  return vec3 (std::cos (lon) * std::cos (lat), std::sin (lon) * std::cos (lat), std::sin (lat));
}

// Signed area of a spherical triangle (Van Oosterom & Strackee)
static double sphArea (const vec3 & a, const vec3 & b, const vec3 & c)
{
  // Differences avoid the cancellation of a . (b x c) for very small triangles
  double num = glm::dot (a, glm::cross (b - a, c - a));
  double den = 1.0 + glm::dot (a, b) + glm::dot (b, c) + glm::dot (c, a);
  return 2.0 * std::atan2 (num, den);
}

// Stereographic projection seen from -c
struct proj_t
{
  vec3 c, e1, e2;
  void init (const vec3 & c0)
  {
    c = glm::normalize (c0);
    vec3 a = std::fabs (c.z) < 0.9 ? vec3 (0, 0, 1) : vec3 (1, 0, 0);
    e1 = glm::normalize (glm::cross (a, c));
    e2 = glm::cross (c, e1);
  }
  // returns false if p is (nearly) at the antipode of c
  bool fwd (const vec3 & p, double * x, double * y) const
  {
    double d = 1.0 + glm::dot (p, c);
    *x = 2.0 * glm::dot (p, e1) / d;
    *y = 2.0 * glm::dot (p, e2) / d;
    return d > 1e-9;
  }
  vec3 inv (double x, double y) const
  {
    double r2 = x * x + y * y;
    double t = 4.0 / (4.0 + r2);
    return glm::normalize (t * x * e1 + t * y * e2 + (2.0 * t - 1.0) * c);
  }
};

// Planar point in polygon (lon/lat), used to assign holes to outer rings
static bool pipLonLat (double x, double y, const ring_t & r)
{
  bool in = false;
  for (int i = 0, j = r.len - 1; i < r.len; j = i++)
    {
      double xi = lonlat[2*(r.off+i)], yi = lonlat[2*(r.off+i)+1];
      double xj = lonlat[2*(r.off+j)], yj = lonlat[2*(r.off+j)+1];
      if (((yi > y) != (yj > y)) && (x < (xj - xi) * (y - yi) / (yj - yi) + xi))
        in = ! in;
    }
  return in;
}

static bool pip2 (double x, double y, const std::vector<double> & P, int b, int n)
{
  bool in = false;
  for (int i = 0, j = n - 1; i < n; j = i++)
    {
      double xi = P[2*(b+i)], yi = P[2*(b+i)+1];
      double xj = P[2*(b+j)], yj = P[2*(b+j)+1];
      if (((yi > y) != (yj > y)) && (x < (xj - xi) * (y - yi) / (yj - yi) + xi))
        in = ! in;
    }
  return in;
}

static void readShapefile (const std::string & file, std::vector<poly_t> * polys)
{
  std::string path = file.substr (0, file.length () - 4);
  SHPHandle fp = SHPOpen (path.c_str (), "r");
  if (fp == nullptr)
    throw std::runtime_error ("Cannot open " + file);

  int nentities, ntype;
  double minb[4], maxb[4];
  SHPGetInfo (fp, &nentities, &ntype, minb, maxb);

  // Exactly like glGrib::ShapeLib::read : float deg2rad (glGrib/Trigonometry.h), product in
  // double, then rounded to float. A double deg2rad changes the last bit of some coordinates,
  // and that is enough to change EarCut's result on Honshu (i_L1 102218).
  const float deg2rad = M_PI / 180.0f;

  for (int i = 0; i < nentities; i++)
    {
      SHPObject * shape = SHPReadObject (fp, i);
      int nparts = shape->nParts > 0 ? shape->nParts : 1;
      std::vector<ring_t> outers, holes;
      for (int p = 0; p < nparts; p++)
        {
          int j1 = shape->nParts > 0 ? shape->panPartStart[p] : 0;
          int j2 = (p + 1 < shape->nParts) ? shape->panPartStart[p+1] : shape->nVertices;
          ring_t r;
          r.off = lonlat.size () / 2;
          r.len = j2 - j1 - 1;  // Same as Land::layer_t::triangulate
          double s = 0;
          for (int j = j1; j < j2; j++)
            {
              lonlat.push_back ((float)(shape->padfX[j] * deg2rad)); // as glgrib
              lonlat.push_back ((float)(shape->padfY[j] * deg2rad));
              if (j + 1 < j2)
                s += shape->padfX[j] * shape->padfY[j+1] - shape->padfX[j+1] * shape->padfY[j];
            }
          r.sarea_ll = 0.5 * s;
          if (r.len < 3)
            continue;
          if (r.sarea_ll > 0 && nparts > 1)
            holes.push_back (r);
          else
            outers.push_back (r);
        }
      SHPDestroyObject (shape);

      size_t first = polys->size ();
      for (auto & o : outers)
        {
          poly_t p;
          p.rank = o.off;
          p.outer = o;
          polys->push_back (p);
        }
      for (auto & h : holes)
        {
          double x = lonlat[2*h.off], y = lonlat[2*h.off+1];
          size_t k = first;
          if (polys->size () - first > 1)
            for (size_t q = first; q < polys->size (); q++)
              if (pipLonLat (x, y, (*polys)[q].outer))
                {
                  k = q;
                  break;
                }
          if (k < polys->size ())
            (*polys)[k].holes.push_back (h);
        }
    }

  SHPClose (fp);
}

struct work_t
{
  // Local vertices : outer ring, then holes, then Steiner points
  std::vector<vec3> V;
  std::vector<int> ringb, ringn;  // local begin & length of each ring
  std::vector<int> tri;           // local indices, 3 per triangle
  int nP = 0;                     // number of input vertices
};

static void gatherRings (const poly_t & p, bool useHoles, work_t * w)
{
  auto add = [&] (const ring_t & r)
  {
    w->ringb.push_back (w->V.size ());
    w->ringn.push_back (r.len);
    for (int j = 0; j < r.len; j++)
      w->V.push_back (xyzOf (r.off + j));
  };
  add (p.outer);
  if (useHoles)
    for (auto & h : p.holes)
      add (h);
  w->nP = w->V.size ();
}

static void runEarcut (const std::string & method, const poly_t & p, work_t * w, bool inPar)
{
  // Outer ring only : EarCut does not handle holes
  int len = p.outer.len;
  std::vector<unsigned int> ind (3 * (len - 2));
  int indl = ind.size ();
  bool openmp = (! inPar) || (len >= 300);
  openmp = openmp && (len >= 300);
  if (method == "earcut")
    glGrib::EarCut::processRing (lonlat, p.outer.off, len, 0, &indl, &ind, openmp);
  else
    earcutFix (lonlat, p.outer.off, len, 0, &indl, &ind, openmp);
  for (int i = 0; i < indl; i++)
    w->tri.push_back ((int)ind[i] - p.outer.off);
}

static void project (const work_t & w, const proj_t & pr, std::vector<double> * P, int * antipode)
{
  P->resize (2 * w.nP);
  *antipode = 0;
  for (int i = 0; i < w.nP; i++)
    if (! pr.fwd (w.V[i], &(*P)[2*i], &(*P)[2*i+1]))
      (*antipode)++;
}

static void runTriangle (const std::string & method, work_t * w, const proj_t & pr, result_t * res)
{
  double t0 = omp_get_wtime ();
  std::vector<double> P;
  project (*w, pr, &P, &res->antipode);
  // Merge exact duplicates (as for CDT) : with duplicate vertices, Triangle -Y can crash
  // (segfault in scoutsegment/finddirection), depending on its global random seed
  std::map<std::pair<double,double>,int> seen;
  std::vector<int> l2u (w->nP);
  std::vector<int> u2l;
  std::vector<double> PU;
  for (int i = 0; i < w->nP; i++)
    {
      auto key = std::make_pair (P[2*i], P[2*i+1]);
      auto it = seen.find (key);
      if (it == seen.end ())
        {
          int u = u2l.size ();
          seen[key] = u;
          u2l.push_back (i);
          PU.push_back (P[2*i]);
          PU.push_back (P[2*i+1]);
          l2u[i] = u;
        }
      else
        l2u[i] = it->second;
    }
  int nU = u2l.size ();

  std::vector<int> S;
  for (size_t r = 0; r < w->ringb.size (); r++)
    for (int j = 0; j < w->ringn[r]; j++)
      {
        int a = l2u[w->ringb[r] + j], b = l2u[w->ringb[r] + (j + 1) % w->ringn[r]];
        if (a == b)
          continue;
        S.push_back (a);
        S.push_back (b);
      }
  double t1 = omp_get_wtime ();

  struct triangulateio in, out;
  memset (&in, 0, sizeof (in));
  memset (&out, 0, sizeof (out));
  in.numberofpoints = nU;
  in.pointlist = PU.data ();
  in.numberofsegments = S.size () / 2;
  in.segmentlist = S.data ();

  std::string sw = method == "triangle-Y" ? "pzQBPY" : "pzQBP";
  std::vector<char> swc (sw.begin (), sw.end ());
  swc.push_back (0);

  if (setjmp (tri_jmp) != 0)
    {
      res->status = "error";
      return;
    }
  triangulate (swc.data (), &in, &out, nullptr);
  double t2 = omp_get_wtime ();

  std::vector<int> map (out.numberofpoints);
  for (int i = 0; i < out.numberofpoints; i++)
    if (i < nU)
      map[i] = u2l[i];
    else
      {
        map[i] = w->V.size ();
        w->V.push_back (pr.inv (out.pointlist[2*i], out.pointlist[2*i+1]));
      }
  res->nsteiner = out.numberofpoints - nU;

  // Remove triangles inside holes (Triangle only eats the outside from the convex hull)
  for (int t = 0; t < out.numberoftriangles; t++)
    {
      const int * T = &out.trianglelist[3*t];
      bool keep = true;
      if (w->ringb.size () > 1)
        {
          double gx = 0, gy = 0;
          for (int k = 0; k < 3; k++)
            {
              gx += out.pointlist[2*T[k]] / 3.0;
              gy += out.pointlist[2*T[k]+1] / 3.0;
            }
          for (size_t r = 1; r < w->ringb.size (); r++)
            if (pip2 (gx, gy, P, w->ringb[r], w->ringn[r]))
              keep = false;
        }
      if (keep)
        for (int k = 0; k < 3; k++)
          w->tri.push_back (map[T[k]]);
    }
  double t3 = omp_get_wtime ();

  free (out.pointlist);
  free (out.trianglelist);
  free (out.pointmarkerlist);
  free (out.segmentlist);
  free (out.segmentmarkerlist);

  res->t_proj = t1 - t0;
  res->t_tri = (t2 - t1) + (t3 - t2);
}

static void runCDT (const std::string & method, work_t * w, const proj_t & pr, result_t * res)
{
  double t0 = omp_get_wtime ();
  std::vector<double> P;
  project (*w, pr, &P, &res->antipode);
  std::vector<CDT::V2d<double> > verts (w->nP);
  for (int i = 0; i < w->nP; i++)
    verts[i] = CDT::V2d<double> (P[2*i], P[2*i+1]);
  std::vector<CDT::Edge> edges;
  for (size_t r = 0; r < w->ringb.size (); r++)
    for (int j = 0; j < w->ringn[r]; j++)
      {
        int a = w->ringb[r] + j, b = w->ringb[r] + (j + 1) % w->ringn[r];
        edges.push_back (CDT::Edge (a, b));
      }
  double t1 = omp_get_wtime ();

  // Exact duplicates (e.g. Natural Earth points at the South Pole) must be removed for CDT
  CDT::DuplicatesInfo dup = CDT::RemoveDuplicatesAndRemapEdges (verts, edges);
  // Degenerate edges (both ends merged) must be dropped
  std::vector<CDT::Edge> edges2;
  for (auto & e : edges)
    if (e.v1 () != e.v2 ())
      edges2.push_back (e);

  CDT::IntersectingConstraintEdges::Enum ice = method == "cdt-noX"
    ? CDT::IntersectingConstraintEdges::NotAllowed : CDT::IntersectingConstraintEdges::TryResolve;

  CDT::Triangulation<double> cdt (CDT::VertexInsertionOrder::Auto, ice, 0.0);
  try
    {
      cdt.insertVertices (verts);
      cdt.insertEdges (edges2);
      if (method == "cdt-outer")
        cdt.eraseOuterTriangles ();
      else
        cdt.eraseOuterTrianglesAndHoles ();
    }
  catch (const std::exception & e)
    {
      res->status = "error";
      return;
    }
  double t2 = omp_get_wtime ();

  // Map back : unique vertex index -> original local index ; new vertices -> Steiner
  int nu = verts.size ();
  std::vector<int> u2l (nu, -1);
  for (int i = w->nP - 1; i >= 0; i--)
    u2l[dup.mapping[i]] = i;
  int nV = cdt.vertices.size ();
  std::vector<int> map (nV);
  for (int i = 0; i < nV; i++)
    if (i < nu)
      map[i] = u2l[i];
    else
      {
        map[i] = w->V.size ();
        w->V.push_back (pr.inv (cdt.vertices[i].x, cdt.vertices[i].y));
      }
  res->nsteiner = nV - nu;
  for (auto & t : cdt.triangles)
    for (int k = 0; k < 3; k++)
      w->tri.push_back (map[t.vertices[k]]);
  double t3 = omp_get_wtime ();

  res->t_proj = t1 - t0;
  res->t_tri = (t2 - t1) + (t3 - t2);
}

static void check (const work_t & w, const vec3 & c, result_t * res)
{
  // Ring area : fan from c, summed over outer ring and holes (holes have opposite orientation)
  double aring = 0, aouter = 0;
  for (size_t r = 0; r < w.ringb.size (); r++)
    {
      double a = 0;
      for (int j = 0; j < w.ringn[r]; j++)
        a += sphArea (c, w.V[w.ringb[r]+j], w.V[w.ringb[r]+(j+1)%w.ringn[r]]);
      if (r == 0)
        aouter = a;
      aring += a;
    }
  // The orientation of the output triangles is a convention of each method (all give the
  // opposite of the GSHHS rings) : compare absolute values, and count the triangles whose
  // orientation differs from the orientation of the sum
  (void) aouter;
  double s = 0, sabs = 0;
  int ntri = w.tri.size () / 3;
  std::vector<double> at (ntri);
  for (int t = 0; t < ntri; t++)
    {
      at[t] = sphArea (w.V[w.tri[3*t]], w.V[w.tri[3*t+1]], w.V[w.tri[3*t+2]]);
      s += at[t];
      sabs += std::fabs (at[t]);
    }
  double sgn = s >= 0 ? 1.0 : -1.0;
  int inv = 0;
  for (int t = 0; t < ntri; t++)
    if (at[t] * sgn < -1e-15)
      inv++;
  res->area_err = std::fabs (std::fabs (s) - std::fabs (aring)) / std::fabs (aring);
  res->overlap = (sabs - std::fabs (s)) / std::fabs (aring);
  res->inverted = inv;
  res->area = std::fabs (aring);
  res->inv_area = 0.5 * (sabs - std::fabs (s));

  // Identical points (e.g. repeated consecutive points in Natural Earth lakes) are merged
  // before checking edges : an edge between two copies of a point is not expected
  std::map<std::tuple<double,double,double>,int> pos;
  std::vector<int> canon (w.V.size ());
  for (size_t i = 0; i < w.V.size (); i++)
    {
      auto key = std::make_tuple (w.V[i].x, w.V[i].y, w.V[i].z);
      auto it = pos.find (key);
      canon[i] = it == pos.end () ? (pos[key] = i) : it->second;
    }
  int nUnique = 0;
  for (int i = 0; i < w.nP; i++)
    if (canon[i] == i)
      nUnique++;
  res->expected = nUnique + 2 * (w.ringb.size () - 1) - 2;

  std::set<std::pair<int,int> > E;
  for (int t = 0; t < ntri; t++)
    for (int k = 0; k < 3; k++)
      {
        int a = canon[w.tri[3*t+k]], b = canon[w.tri[3*t+(k+1)%3]];
        E.insert (std::make_pair (std::min (a, b), std::max (a, b)));
      }
  int missing = 0;
  for (size_t r = 0; r < w.ringb.size (); r++)
    for (int j = 0; j < w.ringn[r]; j++)
      {
        int a = canon[w.ringb[r] + j], b = canon[w.ringb[r] + (j + 1) % w.ringn[r]];
        if (a == b)
          continue;
        if (! E.count (std::make_pair (std::min (a, b), std::max (a, b))))
          missing++;
      }
  res->missing = missing;

  // Rings of (practically) zero area : degenerate input, nothing to triangulate
  // (1e-16 sr is about 0.004 m^2)
  if ((res->status.empty () || (res->status == "error")) && (std::fabs (aring) < 1e-16))
    res->status = "degenerate";

  if (res->status.empty ())
    {
      bool good = (res->area_err < 1e-6) && (res->overlap < 1e-6) && (inv == 0);
      if (good && (missing == 0))
        res->status = "ok";
      else if (good && (res->nsteiner > 0))
        res->status = "ok-steiner";
      else if (ntri == 0)
        res->status = "empty";
      else
        res->status = "bad";
    }
}

// Largest angular gap between an edge's great-circle arc and its straight image (stereographic)
static double sagitta (const work_t & w, const proj_t & pr)
{
  double m = 0;
  for (size_t r = 0; r < w.ringb.size (); r++)
    for (int j = 0; j < w.ringn[r]; j++)
      {
        const vec3 & a = w.V[w.ringb[r]+j];
        const vec3 & b = w.V[w.ringb[r]+(j+1)%w.ringn[r]];
        vec3 mid = a + b;
        if (glm::length (mid) < 1e-12)
          continue;
        mid = glm::normalize (mid);
        double xa, ya, xb, yb, xm, ym;
        pr.fwd (a, &xa, &ya);
        pr.fwd (b, &xb, &yb);
        pr.fwd (mid, &xm, &ym);
        double d = std::hypot (xm - 0.5 * (xa + xb), ym - 0.5 * (ya + yb));
        double scale = 0.5 * (1.0 + glm::dot (mid, pr.c));  // plane distance -> angle
        m = std::max (m, d * scale);
      }
  return m;
}

static void dumpPoly (const std::string & dir, const poly_t & p, const work_t & w, const result_t & r)
{
  std::string f = dir + "/poly_" + std::to_string (p.rank) + ".txt";
  FILE * fp = fopen (f.c_str (), "w");
  if (fp == nullptr)
    return;
  fprintf (fp, "R %d %d %zu %s\n", p.rank, w.nP, w.ringb.size (), r.status.c_str ());
  for (size_t q = 0; q < w.ringb.size (); q++)
    fprintf (fp, "B %d %d\n", w.ringb[q], w.ringn[q]);
  for (auto & v : w.V)
    fprintf (fp, "V %.9f %.9f\n", std::atan2 (v.y, v.x) * 180.0 / M_PI,
             std::asin (std::max (-1.0, std::min (1.0, v.z))) * 180.0 / M_PI);
  for (size_t t = 0; t < w.tri.size () / 3; t++)
    fprintf (fp, "T %d %d %d\n", w.tri[3*t], w.tri[3*t+1], w.tri[3*t+2]);
  fclose (fp);
}

static result_t processPoly (const std::string & method, const poly_t & p, bool useHoles,
                             bool doCheck, bool inPar, const std::string & dumpDir)
{
  result_t res;
  work_t w;
  static const bool verbose = std::getenv ("TRI_DRIVER_VERBOSE") != nullptr;
  if (verbose)
    fprintf (stderr, "poly %d N=%d holes=%zu\n", p.rank, p.outer.len, p.holes.size ());
  gatherRings (p, useHoles && (method.substr (0, 6) != "earcut"), &w);
  res.N = w.nP;
  res.nholes = w.ringb.size () - 1;

  // Projection centre : normalized mean of the outer ring
  vec3 c (0.0);
  for (int j = 0; j < w.ringn[0]; j++)
    c += w.V[j];
  if (glm::length (c) < 1e-9)
    c = vec3 (0, 0, -1);
  proj_t pr;
  pr.init (c);

  double t0 = omp_get_wtime ();
  if (method.substr (0, 6) == "earcut")
    {
      runEarcut (method, p, &w, inPar);
      res.t_tri = omp_get_wtime () - t0;
    }
  else if (method.substr (0, 8) == "triangle")
    runTriangle (method, &w, pr, &res);
  else
    runCDT (method, &w, pr, &res);

  res.ntri = w.tri.size () / 3;
  res.expected = w.nP + 2 * res.nholes - 2;  // refined in check () (identical points merged)

  if (doCheck)
    {
      check (w, pr.c, &res);
      res.max_sagitta = sagitta (w, pr);
    }
  else if (res.status.empty ())
    res.status = "-";

  if (! dumpDir.empty ())
    dumpPoly (dumpDir, p, w, res);
  return res;
}

int main (int argc, char * argv[])
{
  if (argc < 3)
    {
      fprintf (stderr, "Usage: %s file.shp METHOD [par] [nocheck] [noholes] [dump=dir] [rank ...|all|min=N]\n", argv[0]);
      return 1;
    }

  std::string file = argv[1], method = argv[2];
  std::set<std::string> methods = {"earcut", "earcut-fix", "triangle", "triangle-Y", "cdt", "cdt-noX", "cdt-outer"};
  if (! methods.count (method))
    {
      fprintf (stderr, "Unknown method %s\n", method.c_str ());
      return 1;
    }

  bool par = false, doCheck = true, useHoles = true, all = false;
  int nmin = -1;
  std::string dumpDir;
  std::set<int> ranks;
  for (int i = 3; i < argc; i++)
    {
      std::string a = argv[i];
      if (a == "par") par = true;
      else if (a == "nocheck") doCheck = false;
      else if (a == "noholes") useHoles = false;
      else if (a == "all") all = true;
      else if (a.substr (0, 4) == "min=") nmin = std::atoi (a.c_str () + 4);
      else if (a.substr (0, 5) == "dump=") dumpDir = a.substr (5);
      else ranks.insert (std::atoi (a.c_str ()));
    }

  std::vector<poly_t> polys;
  readShapefile (file, &polys);

  std::vector<int> sel;
  for (size_t k = 0; k < polys.size (); k++)
    if (all || ranks.count (polys[k].rank) || ((nmin > 0) && (polys[k].outer.len >= nmin)))
      sel.push_back (k);

  std::vector<result_t> res (sel.size ());

  double t0 = omp_get_wtime ();
  if (par)
    {
      // Like Land::layer_t::triangulate : big rings first, serially (EarCut uses OpenMP inside),
      // then small rings in a parallel loop
      std::vector<int> ord (sel.size ());
      for (size_t l = 0; l < ord.size (); l++) ord[l] = l;
      std::sort (ord.begin (), ord.end (), [&] (int a, int b)
                 { return polys[sel[a]].outer.len > polys[sel[b]].outer.len; });
      size_t k = 0;
      for (; k < ord.size (); k++)
        {
          if (polys[sel[ord[k]]].outer.len < 300)
            break;
          res[ord[k]] = processPoly (method, polys[sel[ord[k]]], useHoles, doCheck, false, dumpDir);
        }
#pragma omp parallel for schedule (dynamic, 16)
      for (size_t l = k; l < ord.size (); l++)
        res[ord[l]] = processPoly (method, polys[sel[ord[l]]], useHoles, doCheck, true, dumpDir);
    }
  else
    {
      for (size_t l = 0; l < sel.size (); l++)
        res[l] = processPoly (method, polys[sel[l]], useHoles, doCheck, false, dumpDir);
    }
  double t1 = omp_get_wtime ();

  std::map<std::string,int> count;
  long nsteiner = 0, ntri = 0, nholes = 0, nanti = 0;
  double tproj = 0, ttri = 0, sag = 0;
  for (size_t l = 0; l < sel.size (); l++)
    {
      const result_t & r = res[l];
      printf ("%d %d %d %d %d %d %s %.3e %.3e %d %d %d %.6e %.6e %.3e %.6e %.3e\n", polys[sel[l]].rank, r.N, r.nholes,
              r.ntri, r.expected, r.nsteiner, r.status.c_str (), r.area_err, r.overlap, r.inverted, r.missing,
              r.antipode, r.t_proj, r.t_tri, r.max_sagitta, r.area, r.inv_area);
      count[r.status]++;
      nsteiner += r.nsteiner;
      ntri += r.ntri;
      nholes += r.nholes;
      nanti += r.antipode;
      tproj += r.t_proj;
      ttri += r.t_tri;
      sag = std::max (sag, r.max_sagitta);
    }
  printf ("# method %s polys %zu holes %ld triangles %ld steiner %ld antipode %ld wall %.4f sum_proj %.4f sum_tri %.4f max_sagitta %.3e threads %d",
          method.c_str (), sel.size (), nholes, ntri, nsteiner, nanti, t1 - t0, tproj, ttri, sag,
          par ? omp_get_max_threads () : 1);
  for (auto & kv : count)
    printf (" %s=%d", kv.first.c_str (), kv.second);
  printf ("\n");

  return 0;
}
