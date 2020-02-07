
#pragma once

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

  static align_t str2align (const std::string & str)
  {
#define S2A(x) do { if (str == #x) return x; } while (0)
    S2A (C);  S2A (N);  S2A (S); 
    S2A (W);  S2A (E);  S2A (NE); 
    S2A (SE); S2A (NW); S2A (SW);
#undef S2A
    return C;
  }

  glgrib_string & operator= (const glgrib_string &);
  void setup2D (const_glgrib_font_ptr, const std::string &, float, 
               float, float = 1.0f, align_t = SW);
  void setup2D (const_glgrib_font_ptr, const std::vector<std::string> &, 
	       const std::vector<float> & = std::vector<float>{}, 
	       const std::vector<float> & = std::vector<float>{}, 
	       float = 1.0f, align_t = SW);
  void setup3D (const_glgrib_font_ptr, const std::vector<std::string> &, 
	       const std::vector<float> & = std::vector<float>{}, const std::vector<float> & = std::vector<float>{},
	       const std::vector<float> & = std::vector<float>{}, const std::vector<float> & = std::vector<float>{},
	       float = 1.0f, align_t = SW);
  void setup (const_glgrib_font_ptr, const std::vector<std::string> &, 
             const std::vector<float> &, const std::vector<float> &, 
	     float = 1.0f, align_t = SW,
	     const std::vector<float> & = std::vector<float>{}, const std::vector<float> & = std::vector<float>{},
	     const std::vector<float> & = std::vector<float>{}, const std::vector<float> & = std::vector<float>{});
  void setup2D (const_glgrib_font_ptr, const std::vector<std::string> &, float, 
               float, float = 1.0f, align_t = SW);
  void render (const glm::mat4 &) const;
  void render (const glgrib_view &) const;
  void setForegroundColor (const glgrib_option_color & color)
  {
    d.color0[0] = color.r / 255.0f; 
    d.color0[1] = color.g / 255.0f; 
    d.color0[2] = color.b / 255.0f;
    d.color0[3] = color.a / 255.0f;
  }
  void setBackgroundColor (const glgrib_option_color & color)
  {
    d.color1[0] = color.r / 255.0f; 
    d.color1[1] = color.g / 255.0f; 
    d.color1[2] = color.b / 255.0f;
    d.color1[3] = color.a / 255.0f;
  }
  void setForegroundColor (float r, float g, float b, float a = 1.0f) 
    { d.color0[0] = r; d.color0[1] = g; d.color0[2] = b; d.color0[3] = a; }
  void setBackgroundColor (float r, float g, float b, float a = 1.0f) 
    { d.color1[0] = r; d.color1[1] = g; d.color1[2] = b; d.color1[3] = a; }
  ~glgrib_string ();
  void update (const std::vector<std::string> &);
  void update (const std::string &);
  void clear ();
  bool ready = false;

  void setupVertexAttributes ();

  void setShared (bool);
  void setChange (bool);
  bool isReady () const { return ready; }

  void setScaleXYZ (float s)
  {
    d.scaleXYZ = s;
  }

private:
  GLuint VertexArrayID;
  struct 
  {
    bool shared = false;
    bool change = true;
    std::vector<std::string> data;
    std::vector<float> x, y;       // Position of letters vertices
    std::vector<float> X, Y, Z, A; // Position & angle of each letter on the sphere
    align_t align;
    float color0[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float color1[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float scale;
    float scaleXYZ = 1.0f;
    int len;    // Total number of characters
    glgrib_opengl_buffer_ptr xyzbuffer, vertexbuffer, letterbuffer;
    const_glgrib_font_ptr font = NULL; 
  } d;
};
