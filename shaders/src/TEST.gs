#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

out vec4 fragmentColor;

uniform mat4 MVP;

void main () 
{    
  fragmentColor = vec4 (1.0f, 0.0f, 0.0f, 1.0f);

  gl_Position = gl_in[0].gl_Position;  
  gl_Position.x = +gl_Position.x;
  gl_Position.y = +gl_Position.y;
  gl_Position.z = +gl_Position.z;
  gl_Position = MVP * gl_Position;
  EmitVertex ();

  gl_Position = gl_in[1].gl_Position;  
  gl_Position.x = +gl_Position.x;
  gl_Position.y = +gl_Position.y;
  gl_Position.z = +gl_Position.z;
  gl_Position = MVP * gl_Position;  
  EmitVertex ();

  gl_Position = gl_in[2].gl_Position;  
  gl_Position.x = +gl_Position.x;
  gl_Position.y = +gl_Position.y;
  gl_Position.z = +gl_Position.z;
  gl_Position = MVP * gl_Position;  
  EmitVertex ();

  EndPrimitive ();


  fragmentColor = vec4 (0.0f, 1.0f, 0.0f, 1.0f);

  gl_Position = gl_in[2].gl_Position;  
  gl_Position.x = -gl_Position.x;
  gl_Position.y = -gl_Position.y;
  gl_Position.z = -gl_Position.z;
  gl_Position = MVP * gl_Position;
  EmitVertex ();

  gl_Position = gl_in[1].gl_Position;  
  gl_Position.x = -gl_Position.x;
  gl_Position.y = -gl_Position.y;
  gl_Position.z = -gl_Position.z;
  gl_Position = MVP * gl_Position;  
  EmitVertex ();

  gl_Position = gl_in[0].gl_Position;  
  gl_Position.x = -gl_Position.x;
  gl_Position.y = -gl_Position.y;
  gl_Position.z = -gl_Position.z;
  gl_Position = MVP * gl_Position;  
  EmitVertex ();

  EndPrimitive ();


}  
