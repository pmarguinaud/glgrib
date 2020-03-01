#pragma once

#include "glGribGeometry.h"
#include "glGribTrigonometry.h"
#include "glGribOptions.h"
#include "glGribHandle.h"

class glGribGeometryLambert : public glGribGeometry
{
public:
  static const double a;
  
  class latlon_t
  {
  public:
    latlon_t () {}
    latlon_t (double _lon, double _lat) : lon (_lon), lat (_lat) {}
    double lat = 0.0, lon = 0.0;
  };
  class latlon_j_t
  {
  public:
    latlon_j_t (const latlon_t & _V, const latlon_t & _A, const latlon_t & _B) 
      : V (_V), A (_A), B (_B) {}
    latlon_t V, A, B;
  };
  class xy_t 
  {
  public:
    xy_t () {}
    xy_t (double _x, double _y) : x (_x), y (_y) {}
    double x = 0.0, y = 0.0;
    xy_t operator+ (const xy_t & p)
    {
      return xy_t (x + p.x, y + p.y);
    }
    xy_t operator- (const xy_t & p)
    {
      return xy_t (x - p.x, y - p.y);
    }
  };
  class xy_j_t
  {
  public:
    xy_j_t (const xy_t & _V, const xy_t & _A, const xy_t & _B) 
      : V (_V), A (_A), B (_B) {}
    xy_t V, A, B;
  };
  class rtheta_t
  {
  public:
    rtheta_t (double _r, double _theta) : r (_r), theta (_theta) {}
    double r, theta;
  };
  class rtheta_j_t
  {
  public:
    rtheta_j_t (const rtheta_t & _V, const rtheta_t & _A, const rtheta_t & _B)
      : V (_V), A (_A), B (_B) {}
    rtheta_t V, A, B;
  };

  static inline double sign (const double & a, const double & b)
  {
    return b > 0 ? +fabs (a) : -fabs (a);
  }
  
  static inline double dist2ref (const latlon_t & pt_coord, const latlon_t & ref_coord)
  {
    double z = pt_coord.lon - ref_coord.lon;
    z = z - sign (pi, z) * (1.0 + sign (1.0, fabs (z) - pi));
    return -z * sign (1.0, z - pi);
  }
  
  class proj_t
  {
  public:
    proj_t () {}
    proj_t (double lon, double lat, double _pole) : ref_pt (lon, lat), pole (_pole)
    {
      kl = pole * sin (ref_pt.lat);
      r_equateur = a * pow (cos (ref_pt.lat), 1.0 - kl) * pow (1.0 + kl, kl) / kl;
    }
    latlon_t ref_pt;
    double pole;
    double r_equateur;
    double kl;

    latlon_j_t rthetaToLatlon (const rtheta_j_t & pt_rtheta) const
    {
      return latlon_j_t 
               (
                 latlon_t 
                   (
                     ref_pt.lon + pt_rtheta.V.theta / kl,
                     pole * ((pi / 2.0) - 2.0 * atan (pow (pt_rtheta.V.r / r_equateur, 1.0 / kl)))
                   ), 
                 latlon_t 
                   (
                     pt_rtheta.A.theta / kl,
                     pole * (-2.0 * pow (pt_rtheta.V.r, 1.0 / kl - 1.0) * pt_rtheta.A.r) /
                     (kl * pow (r_equateur, 1.0 / kl) * (1.0 + pow (pt_rtheta.V.r / r_equateur, 2.0 / kl)))
                   ), 
                 latlon_t 
                   (
                     pt_rtheta.B.theta / kl,
                     pole * (-2.0 * pow (pt_rtheta.V.r, 1.0 / kl - 1.0) * pt_rtheta.B.r) /
                     (kl * pow (r_equateur, 1.0 / kl) * (1.0 + pow (pt_rtheta.V.r / r_equateur, 2.0 / kl)))
                   )
               );
    }
    latlon_t rthetaToLatlon (const rtheta_t & pt_rtheta) const 
    {
      return latlon_t (ref_pt.lon + pt_rtheta.theta / kl,
                       pole * ((pi / 2.0) - 2.0 * atan(pow (pt_rtheta.r / r_equateur, 1.0 / kl))));
    }
    rtheta_j_t xy_to_rtheta (const xy_j_t & pt_xy) const
    {
      double r2 = pt_xy.V.x * pt_xy.V.x + pt_xy.V.y * pt_xy.V.y;
      double r = sqrt (r2);
      double dr_dA = (pt_xy.V.x * pt_xy.A.x + pt_xy.V.y * pt_xy.A.y) / r;
      double dr_dB = (pt_xy.V.x * pt_xy.B.x + pt_xy.V.y * pt_xy.B.y) / r;
      double tatng;
      double theta, dtheta_dA, dtheta_dB;
   
   
      if (pt_xy.V.y == 0.0)
        tatng = (pt_xy.V.x == 0.0) ? pi : sign (pi / 2.0, -pole * pt_xy.V.x);
      else
        tatng = atan (-pole * (pt_xy.V.x / pt_xy.V.y));

      theta = pi * sign (1.0, pt_xy.V.x) * (sign (0.5, pole * pt_xy.V.y) + 0.5) + tatng;

      if (r2 > 0.0)
        {
          dtheta_dA = -pole * (pt_xy.V.y * pt_xy.A.x - pt_xy.V.x * pt_xy.A.y) / r2;
          dtheta_dB = -pole * (pt_xy.V.y * pt_xy.B.x - pt_xy.V.x * pt_xy.B.y) / r2;
        }
      else
        {
          dtheta_dA = 0.0;
          dtheta_dB = 0.0;
        }
   
      return rtheta_j_t (rtheta_t (r, theta), rtheta_t (dr_dA, dtheta_dA), rtheta_t (dr_dB, dtheta_dB));
    }
    rtheta_t xy_to_rtheta (const xy_t & pt_xy) const 
    {
      double r = sqrt(pt_xy.x * pt_xy.x + pt_xy.y * pt_xy.y);
      double tatng;
      double theta;
    
      if (pt_xy.y == 0.0) 
        tatng = (pt_xy.x == 0.0) ? pi : sign (pi / 2.0, -pole * pt_xy.x);
      else
        tatng = atan (-pole * (pt_xy.x / pt_xy.y));
      theta = pi * sign (1.0, pt_xy.x) * (sign (0.5, pole * pt_xy.y) + 0.5) + tatng;
    
      return rtheta_t (r, theta);
    }
    latlon_j_t xy_to_latlon (const xy_j_t & pt_xy) const
    {
      return rthetaToLatlon (xy_to_rtheta (pt_xy));
    }
    latlon_t xy_to_latlon (const xy_t & pt_xy) const 
    {
      return rthetaToLatlon (xy_to_rtheta (pt_xy));
    }
    rtheta_t latlonToRtheta (const latlon_t & pt_coord) const 
    {
      return rtheta_t 
    	  (r_equateur * pow (tan ((pi / 4.0) - ((pole * pt_coord.lat) / 2.0)), kl),
               kl * dist2ref (pt_coord, ref_pt));
    }
    xy_t rthetaToXy (const rtheta_t & pt_rtheta) const 
    {
      return xy_t (pt_rtheta.r * sin (pt_rtheta.theta), -pole * pt_rtheta.r * cos (pt_rtheta.theta));
    }
    xy_t latlonToXy (const latlon_t & pt_coord) const 
    {
      return rthetaToXy (latlonToRtheta (pt_coord));
    }
  
  };

  bool isEqual (const glGribGeometry &) const override;
  void getPointNeighbours (int, std::vector<int> *) const override;
  std::string md5 () const override;
  int latlon2index (float, float) const override;
  void index2latlon (int, float *, float *) const override;
  glGribGeometryLambert (glgrib_handle_ptr);
  void setup (glgrib_handle_ptr, const glGribOptionsGeometry &) override;
  int size () const override;
  virtual ~glGribGeometryLambert ();
  void applyNormScale (float *) const override {}
  void applyUVangle (float *) const override;
  void sample (unsigned char *, const unsigned char, const int) const override;
  void sampleTriangle (unsigned char *, const unsigned char, const int) const override;
  float resolution (int = 0) const override;
  void getTriangleVertices (int, int [3]) const override;
  void getTriangleNeighbours (int, int [3], int [3], glm::vec3 [3]) const override;
  void getTriangleNeighbours (int, int [3], int [3], glm::vec2 [3]) const override;
  bool triangleIsEdge (int) const override;
  int getTriangle (float, float) const override;
  glm::vec2 xyz2conformal (const glm::vec3 &) const override;
  glm::vec3 conformal2xyz (const glm::vec2 &) const override;
  glm::vec2 conformal2latlon (const glm::vec2 &) const override;
  void fixPeriodicity (const glm::vec2 &, glm::vec2 *, int) const override {}
  float getLocalMeshSize (int) const override;
  void getView (glGribView *) const override;
  void setProgramParameters (glGribProgram *) const override;
  void setupFrame ();
private:
  void getTriangleNeighboursXY (int, int [3], int [3], xy_t [4]) const;
  void setupCoordinates ();
  long int Nx, Ny, Nux, Nuy, projectionCentreFlag;
  double LaDInDegrees, LoVInDegrees, DxInMetres, DyInMetres;
  proj_t p_pj;
  xy_t center_xy;
};

