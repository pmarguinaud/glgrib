uniform bool do_alpha = false;
uniform float posmax = 0.97;
uniform float width = 0.005;
uniform float height_scale = 0.05;

void getLineVertex (vec2 vertexLonLat0, vec2 vertexLonLat1, vec2 vertexLonLat2,
                    float vertexHeight0, float vertexHeight1, float c0,
                    bool calpha, float dist0, float dist1, out vec3 pos, 
                    out float dist, inout float alpha)
{
  vec3 vertexPos0 = posFromLonLat (vertexLonLat0);
  vec3 vertexPos1 = posFromLonLat (vertexLonLat1);
  vec3 vertexPos2 = posFromLonLat (vertexLonLat2);

  vec3 vertexPos;
  float height;

  vec3 t0 = normalize (vertexPos1 - vertexPos0);
  vec3 t1 = normalize (vertexPos2 - vertexPos1);


  if ((gl_VertexID == 0) || (gl_VertexID == 2))
    {
      vertexPos = vertexPos0;
      height    = vertexHeight0;
    }
  else if ((gl_VertexID == 1) || (gl_VertexID == 3) || (gl_VertexID == 5) || (gl_VertexID == 4))
    {
      vertexPos = vertexPos1;  
      height    = vertexHeight1;
    }

  vec3 p = normalize (vertexPos);
  vec3 n0 = cross (t0, p);
  vec3 n1 = cross (t1, p);

  float c = c0 * width / scalingFactor (p);

  if ((gl_VertexID >= 4) && (dot (cross (n0, n1), vertexPos) < 0.))
    c = 0.0;

  if (gl_VertexID == 2)
    vertexPos = vertexPos + c * n0;
  if (gl_VertexID == 3)
    vertexPos = vertexPos + c * n0;
  if (gl_VertexID == 4)
    vertexPos = vertexPos + c * normalize (n0 + n1);
  if (gl_VertexID == 5)
    vertexPos = vertexPos + c * n1;

  vec3 normedPos = compNormedPos (vertexPos);
  pos = compProjedPos (vertexPos, normedPos);

  if (calpha)
    {
      const float threshold = 0.95;
      alpha = min (min (threshold, length (vertexPos0)), 
                   min (threshold, length (vertexPos1))) / threshold;
    }

  if (proj == XYZ)
    {
      pos = scalePosition (pos, normedPos, scale0);
      pos = pos * (1.0f + height_scale * height);
    }
  else
    {
      if ((proj == LATLON) || (proj == MERCATOR))
      if ((pos.y < -posmax) || (+posmax < pos.y))
        {
          pos.x = -0.1;
          if (do_alpha)
            alpha = 0.0;
	}
      if (proj == LATLON)
      if ((pos.z > +0.49) || (pos.z < -0.49))
        alpha = 0.0;

      if (proj == POLAR_SOUTH)
        pos.x = pos.x - 0.005;
      else
        pos.x = pos.x + 0.005;
    }

  if ((gl_VertexID == 0) || (gl_VertexID == 2))
    {
      dist = dist0;
    }
  else
    {
      dist = dist1;
    }
}


