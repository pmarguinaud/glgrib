#ifndef _GLGRIB_PROGRAM_H
#define _GLGRIB_PROGRAM_H

#include "glgrib_opengl.h"
#include "glgrib_options.h"
#include <string>

class glgrib_program
{
public:
  typedef enum 
  {
    NONE=-1,
    RGBA=0,
    RGB=1,
    MONO=2,
    RGB_FLAT=3,
    RGB_POSITION_SCALE=4,
    GRADIENT_FLAT_SCALE_SCALAR=5,
    FLAT_TEX=6,
    GRADIENT_FLAT_SCALE_VECTOR=7,
    CONTOUR=8,
    FONT=9,
    IMAGE=10,
    POINTS=11,
    SIZE=12,
  } kind;

  static const std::string emptyshader;

  void set1f (const std::string &, float);
  void set1fv (const std::string &, const float *, int = 1);
  void set1iv (const std::string &, const int *, int = 1);
  void set1i (const std::string &, int);
  void set3fv (const std::string &, const float *, int = 1);
  void set4fv (const std::string &, const float *, int = 1);
  void setMatrix4fv (const std::string &, const float *, int = 1);

  void compile ();

  void setLight (const glgrib_options_light &);
  glgrib_program (const std::string & fsc, const std::string & vsc, const std::string & gsc = emptyshader) 
     : FragmentShaderCode (fsc), VertexShaderCode (vsc), GeometryShaderCode (gsc) { }
  virtual ~glgrib_program ();
  void use () const;
  std::string FragmentShaderCode;
  std::string VertexShaderCode;
  std::string GeometryShaderCode;
  GLuint programID;
  GLuint matrixID;
  bool loaded = false;
  mutable bool active = false;
};

glgrib_program * glgrib_program_load (glgrib_program::kind);

#endif
