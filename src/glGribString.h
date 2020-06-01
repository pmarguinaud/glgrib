
#pragma once

#include "glGribObject.h"
#include "glGribFont.h"
#include "glGribView.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glGribOpenGL.h"
#include <string>
#include <vector>


namespace glGrib
{

class String 
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

  String & operator= (const String &);
  String () : VAID (this) {}
  String (const String & str) : VAID (this)
  {
    *this = str;
  }

  void setup2D (const_FontPtr, const std::string &, float, 
               float, float = 1.0f, align_t = SW);
  void setup2D (const_FontPtr, const std::vector<std::string> &, 
	       const std::vector<float> & = std::vector<float>{}, 
	       const std::vector<float> & = std::vector<float>{}, 
	       float = 1.0f, align_t = SW, const std::vector<float> & = std::vector<float>{});
  void setup2D (const_FontPtr, const std::vector<std::string> &, 
                const std::vector<float> &, const std::vector<float> &, 
                float, const std::vector<align_t> &, const std::vector<float> &);
  void setup2D (const_FontPtr, const std::vector<std::string> &, float, 
               float, float = 1.0f, align_t = SW);
  void setup3D (const_FontPtr, const std::vector<std::string> &, 
	       const std::vector<float> & = std::vector<float>{}, const std::vector<float> & = std::vector<float>{},
	       const std::vector<float> & = std::vector<float>{}, const std::vector<float> & = std::vector<float>{},
	       float = 1.0f, align_t = SW);
  void setup (const_FontPtr, const std::vector<std::string> &, 
             const std::vector<float> &, const std::vector<float> &, 
	     float = 1.0f, const std::vector<align_t> & = std::vector<align_t>{SW},
	     const std::vector<float> & = std::vector<float>{}, const std::vector<float> & = std::vector<float>{},
	     const std::vector<float> & = std::vector<float>{}, const std::vector<float> & = std::vector<float>{});
  void render (const glm::mat4 &) const;
  void render (const View &) const;

  void setForegroundColor (const OptionColor & color)
  {
    d.color0 = color;
  }
  void setBackgroundColor (const OptionColor & color)
  {
    d.color1 = color;
  }
  void update (const std::vector<std::string> &);
  void update (const std::string &);

  void setupVertexAttributes () const;

  void setShared (bool p)
  {
    d.shared = p;
  }

  void setChange (bool u)
  {
    if (d.ready && u)
      throw std::runtime_error (std::string ("Cannot set attribute change"));
    d.change = u;
  }

  bool isReady () const { return d.ready; }

  void setScaleXYZ (float s)
  {
    d.scaleXYZ = s;
  }

private:
  struct 
  {
    bool shared = false;
    bool change = true;
    bool ready = false;
    std::vector<std::string> data;
    std::vector<float> x, y;       // Position of letters vertices
    std::vector<float> X, Y, Z, A; // Position & angle of each letter on the sphere
    std::vector<align_t> align;
    OptionColor color0 = OptionColor (255, 255, 255, 255);
    OptionColor color1 = OptionColor (  0,   0,   0,   0);
    float scale;
    float scaleXYZ = 1.0f;
    int len;    // Total number of characters
    OpenGLBufferPtr<float> xyzbuffer, vertexbuffer, letterbuffer;
    const_FontPtr font = nullptr; 
  } d;
  OpenGLVertexArray<String> VAID;
  friend class String2D;
};

class String2D : public Object2D
{
public:
  void render (const glm::mat4 &) const;
  void setup (const_FontPtr, const std::string &, float, 
              float, float = 1.0f, String::align_t = String::SW);
  void setup (const_FontPtr, const std::vector<std::string> &, 
	      const std::vector<float> & = std::vector<float>{}, 
	      const std::vector<float> & = std::vector<float>{}, 
	      float = 1.0f, String::align_t = String::SW,  
              const std::vector<float> & = std::vector<float>{});
  void setup (const_FontPtr, const std::vector<std::string> &, 
              const std::vector<float> &, const std::vector<float> &, 
              float, const std::vector<String::align_t> &, const std::vector<float> &);
  void setup (const_FontPtr, const std::vector<std::string> &, float, 
              float, float = 1.0f, String::align_t = String::SW);
  void setForegroundColor (const OptionColor & color)
  {
    str.setForegroundColor (color);
  }
  void setBackgroundColor (const OptionColor & color)
  {
    str.setBackgroundColor (color);
  }

  void update (const std::vector<std::string> & str)
  {
    this->str.update (str);
  }

  void update (const std::string & str)
  {
    this->str.update (str);
  }

  void setupVertexAttributes () const;

  void setShared (bool p)
  {
    str.setShared (p);
  }

  void setChange (bool u)
  {
    str.setChange (u);
  }

  virtual void reSize (const View &) {}

private:
  String str;
};

}
