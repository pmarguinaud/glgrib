#pragma once

#include "glGribOptionsBase.h"

namespace glGrib
{

class OptionsView : public OptionsBase
{
public:
  DEFINE
  {
    DESC (projection,         Mercator XYZ latlon polar_north polar_south);
    DESC (transformation,     Perspective or orthographic);
    DESC (lon,                Camera longitude);
    DESC (lat,                Camera latitude);
    DESC (fov,                Camera field of view);
    DESC (distance,           Camera distance);
    DESC (center.on,          Center view);
    DESC (clip.on,            Enable Mercator and lat/lon clippling);
    DESC (clip.dlon,          Amount of longitude to clip);
    DESC (clip.dlat,          Amount of latitude to clip);
    DESC (clip.xmin,          Min viewport x coordinate);
    DESC (clip.xmax,          Max viewport x coordinate);
    DESC (clip.ymin,          Min viewport y coordinate);
    DESC (clip.ymax,          Max viewport y coordinate);
    DESC (zoom.on,            Enable zoom with Schmidt transform);
    DESC (zoom.lon,           Longitude of zoom);
    DESC (zoom.lat,           Latitude of zoom);
    DESC (zoom.stretch,       Stretching factor);
  }
  string  projection  = "XYZ";
  string  transformation  = "PERSPECTIVE";
  float  distance  = 6.0; 
  float  lat       = 0.0; 
  float  lon       = 0.0; 
  float  fov       = 20.;
  struct
  {
    bool on = true;
  } center;
  struct
  {
    bool on = false;
    float lon = 2.0f;
    float lat = 46.7f;
    float stretch = 0.5f;
  } zoom;
  struct
  {
    float dlon = 10.0f;
    float dlat =  5.0f;
    float xmin = 0.0f, xmax = 1.0f, ymin = 0.0f, ymax = 1.0f;
    bool on = true;
  } clip;
};

};


