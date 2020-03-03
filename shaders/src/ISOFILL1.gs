#include "version.h"
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in 
#include "ISOFILL1_VS.h"
[3];

out 
#include "ISOFILL1_GS.h"

void main () 
{    
  if ((isofill1_vs[0].geometryColInd == isofill1_vs[1].geometryColInd) 
   && (isofill1_vs[1].geometryColInd == isofill1_vs[2].geometryColInd))
    {
      isofill1_gs.fragmentColInd = isofill1_vs[0].geometryColInd; gl_Position = gl_in[0].gl_Position; EmitVertex ();
      isofill1_gs.fragmentColInd = isofill1_vs[1].geometryColInd; gl_Position = gl_in[1].gl_Position; EmitVertex ();
      isofill1_gs.fragmentColInd = isofill1_vs[2].geometryColInd; gl_Position = gl_in[2].gl_Position; EmitVertex ();
      EndPrimitive ();
    }
}  
