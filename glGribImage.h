#pragma once

#include "glGribView.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glGribOpenGL.h"
#include "glGribOptions.h"
#include <string>
#include <vector>


class glGribImage 
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

  ~glGribImage ();
  
  glGribImage & operator= (const glGribImage &);
  void setupVertexAttributes ();
  void clear ();

  void setup (const glGribOptionsImage &);
  void render (const glm::mat4 &) const;
  const glGribOptionsImage & getOptions () const { return opts; }
private:
  glGribOptionsImage opts;
  bool ready = false;
  glGribOpenGLTexturePtr texture;
  GLuint VertexArrayID = 0;
};

