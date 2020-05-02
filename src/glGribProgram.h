#pragma once

#include "glGribOpenGL.h"
#include "glGribOptions.h"
#include <string>
#include <glm/glm.hpp>

namespace glGrib
{

class Program
{
public:

  typedef enum 
  {
    MONO,
    SCALAR,
    LANDSCAPE,
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
    TICKS,
    FTICKS,
    COLORBAR,
    TEST,
  } kind_t;

  static Program * load (Program::kind_t);

  void set (const std::string & key, bool b)
  {
    set (key, b ? 1 : 0);
  }
  
  void set (const std::string & key, const OptionColor & color)
  {
    set (key, float (color.r) / 255.0f, float (color.g) / 255.0f,
              float (color.b) / 255.0f, float (color.a) / 255.0f);
  }
  
  void set (const std::string & key, float val)
  {
    int id = glGetUniformLocation (programID, key.c_str ());
    if (id != -1)
      glUniform1f (id, val);
  }
  
  void set (const std::string & key, double val)
  {
    set (key, float (val));
  }
  
  void set (const std::string & key, const std::vector<float> & val)
  {
    int id = glGetUniformLocation (programID, key.c_str ());
    if (id != -1)
      glUniform1fv (id, val.size (), val.data ());
  }
  
  void set (const std::string & key, const std::vector<glm::vec4> & val)
  {
    int id = glGetUniformLocation (programID, key.c_str ());
    if (id != -1)
      glUniform4fv (id, val.size (), &val[0][0]);
  }
  
  void set (const std::string & key, const std::vector<int> & val)
  {
    int id = glGetUniformLocation (programID, key.c_str ());
    if (id != -1)
      glUniform1iv (id, val.size (), val.data ());
  }
  
  void set (const std::string & key, long int val)
  {
    set (key, int (val));
  }
  
  void set (const std::string & key, size_t val)
  {
    set (key, int (val));
  }
  
  void set (const std::string & key, int val)
  {
    int id = glGetUniformLocation (programID, key.c_str ());
    if (id != -1)
      glUniform1i (id, val);
  }
  
  void set (const std::string & key, const glm::vec3 & val)
  {
    int id = glGetUniformLocation (programID, key.c_str ());
    if (id != -1)
      glUniform3f (id, val.x, val.y, val.z);
  }
  
  void set (const std::string & key, float x, float y, float z)
  {
    int id = glGetUniformLocation (programID, key.c_str ());
    if (id != -1)
      glUniform3f (id, x, y, z);
  }
  
  void set (const std::string & key, const glm::vec4 & val)
  {
    int id = glGetUniformLocation (programID, key.c_str ());
    if (id != -1)
      glUniform4f (id, val.x, val.y, val.z, val.w);
  }
  
  void set (const std::string & key, float x, float y, float z, float w)
  {
    int id = glGetUniformLocation (programID, key.c_str ());
    if (id != -1)
      glUniform4f (id, x, y, z, w);
  }
  
  void set (const std::string & key, const glm::mat4 & mat)
  {
    int id = glGetUniformLocation (programID, key.c_str ());
    if (id != -1)
      glUniformMatrix4fv (id, 1, GL_FALSE, &mat[0][0]);
  }

  void set (const std::string & key, const glm::mat3 & mat)
  {
    int id = glGetUniformLocation (programID, key.c_str ());
    if (id != -1)
      glUniformMatrix3fv (id, 1, GL_FALSE, &mat[0][0]);
  }

  void set (const OptionsLight &);

  void compile ();

  Program () {}
  Program (const std::string & fsc, const std::string & vsc, const std::string & gsc)
     : FragmentShaderCode (fsc), VertexShaderCode (vsc), GeometryShaderCode (gsc) { }
  Program (const std::string & fsc, const std::string & vsc)
     : FragmentShaderCode (fsc), VertexShaderCode (vsc), GeometryShaderCode ("") { }
  virtual ~Program ();
  void use () const;
  void read (const std::string &);

  std::string FragmentShaderCode;
  std::string VertexShaderCode;
  std::string GeometryShaderCode;
  GLuint programID;
  GLuint matrixID;
  bool loaded = false;
  mutable bool active = false;

  std::string name;
};



}
