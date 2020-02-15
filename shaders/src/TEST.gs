#include "version.h"
layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

void main () 
{    
  gl_Position =  gl_in[0].gl_Position;                              EmitVertex ();
  gl_Position = (gl_in[0].gl_Position+gl_in[1].gl_Position) / 2.0;  EmitVertex ();
  gl_Position = (gl_in[0].gl_Position+gl_in[2].gl_Position) / 2.0;  EmitVertex ();
  gl_Position = (gl_in[0].gl_Position+gl_in[1].gl_Position) / 2.0;  EmitVertex ();

  EndPrimitive ();
}  
