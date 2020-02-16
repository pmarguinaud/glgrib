#pragma once

#include "glgrib_opengl.h"
#include "glgrib_options.h"
#include <string>
#include <glm/glm.hpp>

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
    SCALAR,
    FLAT_TEX,
    VECTOR,
    CONTOUR,
    FONT,
    IMAGE,
    POINTS,
    GRID,
    STREAM,
    SCALAR_POINTS,
    FRAME,
    LAND,
    ISOFILL1,
    ISOFILL2,
    TEST,
  } kind;

  static glgrib_program * load (glgrib_program::kind);

  void set (const std::string &, float);
  void set (const std::string &, const std::vector<float> &);
  void set (const std::string &, const std::vector<int> &);
  void set (const std::string &, int);
  void set (const std::string &, bool);
  void set (const std::string &, const std::vector<glm::vec3> &);
  void set (const std::string &, const glm::vec3 &);
  void set (const std::string &, const std::vector<glm::vec4> &);
  void set (const std::string &, const glm::vec4 &);
  void set (const std::string &, const glm::mat4 &);

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


