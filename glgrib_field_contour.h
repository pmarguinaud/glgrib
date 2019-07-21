#ifndef _GLGRIB_FIELD_CONTOUR_H
#define _GLGRIB_FIELD_CONTOUR_H

#include "glgrib_field.h"

class glgrib_field_contour : public glgrib_field
{
public:
  glgrib_field_contour * clone () const;
  glgrib_field_contour & operator= (const glgrib_field_contour &);
  glgrib_field_contour () { }
  glgrib_field_contour (const glgrib_field_contour &);
  virtual void init (const glgrib_options_field &, int = 0);
  virtual void render (const glgrib_view &, const glgrib_options_light &) const;
  virtual ~glgrib_field_contour ();
  void setupVertexAttributes ();
  virtual void cleanup ();
private:
  class isoline_data_t
  {
  public:
    std::vector<float> xyz; // Position
    std::vector<float> drw; // Flag (=0, hide, =1 show)
    std::vector<float> dis; // Distance 
    double dis_last = 0.;
    void push (const float x, const float y, const float z, const float d = 1.) 
    {
      float D = 0.0f;
      if (d > 0)
        {
          int sz = size (), last = sz - 1;
          if (sz > 0)
            {
              float dx = x - xyz[3*last+0];
              float dy = y - xyz[3*last+1];
              float dz = z - xyz[3*last+2];
              D = dis[last] + sqrt (dx * dx + dy * dy + dz * dz);
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
  class isoline_t
  {
  public:
    float level;
    bool wide = false;
    float width = 0;
    GLuint VertexArrayID;
    glgrib_opengl_buffer_ptr vertexbuffer, normalbuffer, distancebuffer;
    GLuint size;
  };

  std::vector<isoline_t> iso;
  void processTriangle (int, float *, float, bool *, isoline_data_t *);
};

#endif
