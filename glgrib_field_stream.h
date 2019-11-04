#ifndef _GLGRIB_FIELD_STREAM_H
#define _GLGRIB_FIELD_STREAM_H

#include "glgrib_field.h"
#include <bits/stdc++.h> 

class glgrib_field_stream : public glgrib_field
{
public:
  glgrib_field_kind getKind () const 
  {
    return glgrib_field::STREAM;
  }
  glgrib_field_stream * clone () const;
  glgrib_field_stream & operator= (const glgrib_field_stream &);
  glgrib_field_stream () { }
  glgrib_field_stream (const glgrib_field_stream &);
  virtual void setup (glgrib_loader *, const glgrib_options_field &, float = 0);
  virtual void render (const glgrib_view &, const glgrib_options_light &) const;
  virtual ~glgrib_field_stream ();
  void setupVertexAttributes ();
  virtual void clear ();
private:
  class streamline_data_t
  {
  public:
    std::vector<float> xyz; // Position
    std::vector<float> drw; // Flag (=0, hide, =1 show)
    std::vector<float> dis; // Distance 

    void push (const glm::vec3 & xyz, const float d = 1.0f)
    {
      push (xyz.x, xyz.y, xyz.z, d);
    }
    void push (const float x, const float y, const float z, const float d = 1.) 
    {
      double D = 0.0f;
      if (d > 0)
        {
          int sz = size (), last = sz - 1;
          if (sz > 0)
            {
              float x0 = xyz[3*last+0];
              float y0 = xyz[3*last+1];
              float z0 = xyz[3*last+2];
              if ((x0 != 0.0f) || (y0 != 0.0f) || (z0 != 0.0f))
                {
                  float dx = x - x0;
                  float dy = y - y0;
                  float dz = z - z0;
                  D = dis[last] + sqrt (dx * dx + dy * dy + dz * dz);
                }
            }
        }
      xyz.push_back (x);
      xyz.push_back (y);
      xyz.push_back (z);
      drw.push_back (d);
      dis.push_back (D);
    }
    void pop ()
    {
      xyz.pop_back (); 
      xyz.pop_back (); 
      xyz.pop_back (); 
      drw.pop_back (); 
      dis.pop_back (); 
    }
    void clear ()
    {
      xyz.clear (); 
      drw.clear (); 
      dis.clear (); 
    }
    int size ()
    {
      return drw.size ();
    }
  };
  class streamline_t
  {
  public:
    GLuint VertexArrayID;
    glgrib_opengl_buffer_ptr vertexbuffer, normalbuffer, distancebuffer;
    GLuint size;
  };

  streamline_t stream;

  void getFirstPoint (int, const float *, const float *, 
		      glm::vec2 &, glm::vec2 &, glm::vec2 &,
		      std::valarray<float> &, std::valarray<float> &, 
		      int &, int &);
  void processTriangle (int, const float *, const float *, bool *, streamline_data_t *);
  void processTriangle1 (int, const float *, const float *, 
                         const glm::vec2 &, const glm::vec2 &,
                         bool *, float, std::valarray<float>,
                         std::vector<glm::vec3> &);
};

#endif
