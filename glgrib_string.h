#ifndef _GLGRIB_STRING_H
#define _GLGRIB_STRING_H

#include "glgrib_font.h"
#include "glgrib_view.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glgrib_opengl.h"
#include <string>
#include <vector>


class glgrib_string 
{
public:
  static const int CX = 0x01;
  static const int WX = 0x00;
  static const int EX = 0x02;
  static const int CY = 0x01 << 8;
  static const int NY = 0x02 << 8;
  static const int SY = 0x00 << 8;
  typedef enum
    {
      C   = CY | CX, N   = NY | CX, S   = SY | CX, 
      W   = CY | WX, E   = CY | EX, NE  = NY | EX, 
      SE  = SY | EX, NW  = NY | WX, SW  = SY | WX,
    }
  align_t;
  glgrib_string & operator= (const glgrib_string &);
  void init2D (const_glgrib_font_ptr, const std::string &, float, 
               float, float = 1.0f, align_t = SW);
  void init2D (const_glgrib_font_ptr, const std::vector<std::string> &, 
	       const std::vector<float> & = std::vector<float>{}, 
	       const std::vector<float> & = std::vector<float>{}, 
	       float = 1.0f, align_t = SW);
  void init3D (const_glgrib_font_ptr, const std::vector<std::string> &, 
	       const std::vector<float> & = std::vector<float>{}, const std::vector<float> & = std::vector<float>{},
	       const std::vector<float> & = std::vector<float>{}, const std::vector<float> & = std::vector<float>{},
	       float = 1.0f, align_t = SW);
  void init (const_glgrib_font_ptr, const std::vector<std::string> &, 
             const std::vector<float> &, const std::vector<float> &, 
	     float = 1.0f, align_t = SW,
	     const std::vector<float> & = std::vector<float>{}, const std::vector<float> & = std::vector<float>{},
	     const std::vector<float> & = std::vector<float>{}, const std::vector<float> & = std::vector<float>{});
  void init2D (const_glgrib_font_ptr, const std::vector<std::string> &, float, 
               float, float = 1.0f, align_t = SW);
  void render (const glm::mat4 &) const;
  void render (const glgrib_view &) const;
  void setForeGroundColor (float r, float g, float b, float a = 1.0f) 
    { color0[0] = r; color0[1] = g; color0[2] = b; color0[3] = a; }
  void setBackGroundColor (float r, float g, float b, float a = 1.0f) 
    { color1[0] = r; color1[1] = g; color1[2] = b; color1[3] = a; }
  ~glgrib_string ();
  void update (const std::vector<std::string> &);
  void update (const std::string &);
  void cleanup ();
  bool ready = false;
private:
  std::vector<std::string> data;
  std::vector<float> x, y;       // Position of letters vertices
  std::vector<float> X, Y, Z, A; // Position & angle of each letter on the sphere
  align_t align;
  float color0[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  float color1[4] = {0.0f, 0.0f, 0.0f, 0.0f};
  float scale;
  int len;    // Total number of characters
  GLuint VertexArrayID;
  GLuint xyzbuffer, vertexbuffer, letterbuffer;
  const_glgrib_font_ptr font = NULL; 
};

#endif
