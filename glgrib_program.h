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
    RGBA,
    RGB,
    MONO,
    RGB_FLAT,
    RGB_POSITION_SCALE,
    GRADIENT_FLAT_SCALE_SCALAR,
    FLAT_TEX,
    GRADIENT_FLAT_SCALE_VECTOR,
    CONTOUR,
    FONT,
    IMAGE,
    POINTS,
    GRID,
    STREAM,
    SCALAR_POINTS,
    FRAME,
    TEST,
  } kind;

  static glgrib_program * load (glgrib_program::kind);

  void set1f (const std::string &, float);
  void set1fv (const std::string &, const float *, int = 1);
  void set1iv (const std::string &, const int *, int = 1);
  void set1i (const std::string &, int);
  void set3fv (const std::string &, const float *, int = 1);
  void set4fv (const std::string &, const float *, int = 1);
  void setMatrix4fv (const std::string &, const float *, int = 1);

  void compile ();

  void setLight (const glgrib_options_light &);

  glgrib_program () {}
  glgrib_program (const std::string & fsc, const std::string & vsc, const std::string & gsc)
     : FragmentShaderCode (fsc), VertexShaderCode (vsc), GeometryShaderCode (gsc) { }
  glgrib_program (const std::string & fsc, const std::string & vsc)
     : FragmentShaderCode (fsc), VertexShaderCode (vsc), GeometryShaderCode ("") { }
  virtual ~glgrib_program ();
  void use () const;
  void read (const std::string &);

  std::string FragmentShaderCode;
  std::string VertexShaderCode;
  std::string GeometryShaderCode;
  GLuint programID;
  GLuint matrixID;
  bool loaded = false;
  mutable bool active = false;
};


#endif
