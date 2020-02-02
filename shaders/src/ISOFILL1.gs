#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in float geometryColInd[3];
out float fragmentColInd;

void main () 
{    
  if ((geometryColInd[0] == geometryColInd[1]) && (geometryColInd[1] == geometryColInd[2]))
    {
      fragmentColInd = geometryColInd[0]; gl_Position = gl_in[0].gl_Position; EmitVertex ();
      fragmentColInd = geometryColInd[1]; gl_Position = gl_in[1].gl_Position; EmitVertex ();
      fragmentColInd = geometryColInd[2]; gl_Position = gl_in[2].gl_Position; EmitVertex ();
      EndPrimitive ();
    }
}  
