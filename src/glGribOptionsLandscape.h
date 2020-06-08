#pragma once

#include "glGribOptionsBase.h"
#include "glGribOptionsGeometry.h"
#include "glGribOptionsLandscapePosition.h"

namespace glGrib
{

class OptionsLandscape : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on,                  Enable landscape);
    DESC (visible.on,          Landscape is visible);
    DESC (projection,          Projection : LONLAT or WEBMERCATOR);
    DESC (flat.on,             Make Earth flat);
    DESC (path,                Path to landscape image in BMP format);
    DESC (geometry_path,       GRIB files to take geometry from);
    DESC (number_of_latitudes, Number of latitudes used for creating a mesh for the landscape);
    DESC (wireframe.on,        Draw landscape in wireframe mode);
    DESC (scale,               Scale);
    DESC (color,               Color);
    INCLUDE (lonlat.position);
    INCLUDE (geometry);
  }

  struct
  {
    bool on = true;
  } visible;
  string projection = "LONLAT";
  string path  = "landscape/Whole_world_-_land_and_oceans_08000.bmp";
  float  orography  = 0.05;
  string geometry_path = "";
  int number_of_latitudes  = 500;
  struct
  {
    bool on = false;
  } wireframe;
  bool on = false;
  struct
  {
    bool on = true;
  } flat;
  struct
  {
    OptionsLandscapePosition position;
  } lonlat;
  OptionsGeometry geometry;
  float scale = 1.0f;
  OptionColor color = OptionColor ("#00000000");
};

};


