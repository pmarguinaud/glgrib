#pragma once

#include "glGribOptionsBase.h"
#include "glGribOptionsContour.h"
#include "glGribOptionsFont.h"
#include "glGribOptionsGeometry.h"
#include "glGribOptionsIsofill.h"
#include "glGribOptionsMpiview.h"
#include "glGribOptionsPalette.h"
#include "glGribOptionsScalar.h"
#include "glGribOptionsStream.h"
#include "glGribOptionsVector.h"

namespace glGrib
{

class OptionsField : public OptionsBase
{
public:
  DEFINE
  {
    DESC (visible.on,          Field is visible);
    DESC (type,                Field type : SCALAR VECTOR STREAM CONTOUR);
    DESC (user_pref.on,        Lookup field settings in database);
    DESC (path,                List of GRIB files);                    
    DESC (scale,               Scales to be applied to fields);        
    DESC (no_value_pointer.on, Do not keep field values in memory);    
    DESC (diff.on,             Show field difference);
    DESC (hilo.on,             Display low & high);
    DESC (hilo.radius,         High/low radius in degrees);
    INCLUDE (hilo.font);
    INCLUDE (palette);
    INCLUDE (scalar);
    INCLUDE (vector);
    INCLUDE (contour);
    INCLUDE (isofill);
    INCLUDE (stream);
    INCLUDE (geometry);
    INCLUDE_H (mpiview);
    DESC (fatal.on,        Fatal error if field fails to be created);
  }
  std::set<std::string> seen;

  struct
  {
    bool on = true;
  } visible;

  struct 
  {
    bool on = false;
    OptionsFont font;
    float radius = 10.0f;
  } hilo;

  std::string type = "SCALAR";
  struct
  {
    bool on = true;
  } user_pref;
  string_list  path;
  float scale   = 1.0f;
  struct
  {
    bool on = false;
  } no_value_pointer;
  struct
  {
    bool on = false;
  } diff;
  OptionsPalette palette;
  OptionsScalar scalar;
  OptionsVector vector;
  OptionsContour contour;
  OptionsIsofill isofill;
  OptionsStream stream;
  bool parseUnseen (const char *);
  OptionsGeometry geometry;
  OptionsMpiview mpiview;

  struct
  {
    bool on = false;
  } fatal;
};

};


