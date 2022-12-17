
#include "version.h"

uniform mat4 MVP;
uniform float xmin = 0.0f;
uniform float xmax = 1.0f;
uniform float ymin = 0.0f;
uniform float ymax = 1.0f;

uniform float width = 0.1f;

void main()
{
  vec2 pos2;
  if (gl_InstanceID == 0)
    {
      if (gl_VertexID == 0)
        pos2 = vec2 (xmin        , ymin        );
      else if (gl_VertexID == 1)
        pos2 = vec2 (xmax        , ymin        );
      else if (gl_VertexID == 2)
        pos2 = vec2 (xmax        , ymin + width);
      else if (gl_VertexID == 3)
        pos2 = vec2 (xmin        , ymin + width);
    }
  else if (gl_InstanceID == 1)
    {
      if (gl_VertexID == 0)
        pos2 = vec2 (xmin        , ymax - width);
      else if (gl_VertexID == 1)
        pos2 = vec2 (xmax        , ymax - width);
      else if (gl_VertexID == 2)
        pos2 = vec2 (xmax        , ymax        );
      else if (gl_VertexID == 3)
        pos2 = vec2 (xmin        , ymax        );
    }
  else if (gl_InstanceID == 2)
    {
      if (gl_VertexID == 0)
        pos2 = vec2 (xmin        , ymin        );
      else if (gl_VertexID == 1)
        pos2 = vec2 (xmin + width, ymin        );
      else if (gl_VertexID == 2)
        pos2 = vec2 (xmin + width, ymax        );
      else if (gl_VertexID == 3)
        pos2 = vec2 (xmin        , ymax        );
    }
  else if (gl_InstanceID == 3)
    {
      if (gl_VertexID == 0)
        pos2 = vec2 (xmax - width, ymin       );
      else if (gl_VertexID == 1)
        pos2 = vec2 (xmax        , ymin       );
      else if (gl_VertexID == 2)
        pos2 = vec2 (xmax        , ymax       );
      else if (gl_VertexID == 3)
        pos2 = vec2 (xmax - width, ymax       );
    }
  gl_Position =  MVP * vec4 (-0.001, pos2.x, pos2.y, 1.);
}
