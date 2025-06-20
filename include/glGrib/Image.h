#pragma once

#include "glGrib/View.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glGrib/OpenGL.h"
#include "glGrib/Options.h"
#include "glGrib/Object.h"
#include <string>
#include <vector>


namespace glGrib
{

class Image : public Object2D
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


  Image () : VAID (this) {}
  
  void setupVertexAttributes () const
  {
  }

  void setup (const OptionsImage &);
  void render (const glm::mat4 &) const;
  const OptionsImage & getOptions () const { return opts; }

  void reSize (const View &) override
  {
  }

  Object2D::side_t getSide () const override { return Object2D::LEFT; }
private:
  OptionsImage opts;
  OpenGLTexturePtr texture;
  OpenGLVertexArray<Image> VAID;
};


}
