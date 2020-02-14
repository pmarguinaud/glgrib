#pragma once

#include "glgrib_field.h"
#include "glgrib_string.h"

class glgrib_field_contour : public glgrib_field
{
public:
  glgrib_field::kind getKind () const 
  {
    return glgrib_field::CONTOUR;
  }
  glgrib_field_contour * clone () const;
  glgrib_field_contour & operator= (const glgrib_field_contour &);
  glgrib_field_contour () { }
  glgrib_field_contour (const glgrib_field_contour &);
  void setup (glgrib_loader *, const glgrib_options_field &, float = 0) override;
  void render (const glgrib_view &, const glgrib_options_light &) const override;
  virtual ~glgrib_field_contour ();
  void setupVertexAttributes ();
  void clear () override; 
  bool useColorBar () const override { return true; }
  int getSlotMax () const override
  {
    return (int)opts.path.size ();
  }
private:
  class isoline_data_t
  {
  public:
    std::vector<float> lonlat; // Position
    std::vector<float> height; // Height
    std::vector<float> length; // Distance 
    double dis_last = 0.;
    struct
    {
      float x = 0.0f, y = 0.0f, z = 0.0f;
    } last;
    void push (const float x, const float y, const float z, const float h) 
    {
      double D = 0.0f;
      float d = fabs (x) + fabs (y) + fabs (z);
      float lon = 0.0f, lat = 2 * M_PI;
      if (d > 0)
        {
          int sz = size ();
	  lat = asin (z);
	  lon = atan2 (y, x);
          if (sz > 0)
            {
              float dx = x - last.x;
              float dy = y - last.y;
              float dz = z - last.z;
              D = dis_last + sqrt (dx * dx + dy * dy + dz * dz);
            }
        }
      last.x = x;
      last.y = y;
      last.z = z;
      lonlat.push_back (lon);
      lonlat.push_back (lat);
      height.push_back (h);
      length.push_back (D);
      dis_last = D;
    }
    void pop ()
    {
      lonlat.pop_back (); 
      lonlat.pop_back (); 
      height.pop_back (); 
      length.pop_back (); 
    }
    void clear ()
    {
      lonlat.clear (); 
      height.clear (); 
      length.clear (); 
    }
    int size () const
    {
      return length.size ();
    }
  };
  class isoline_t
  {
  public:
    glgrib_string labels;
    float level;
    bool wide = false;
    float width = 0.0f;
    bool dash = false;
    float length = 0.0f;
    glgrib_option_color color;
    std::vector<int> pattern;
    GLuint VertexArrayID;
    glgrib_opengl_buffer_ptr vertexbuffer, heightbuffer, distancebuffer;
    GLuint size;
  };

  std::vector<isoline_t> iso;

  void processTriangle (int, float *, float, float *, float, float, float, bool *, isoline_data_t *);
  void setupLabels (isoline_t *, const isoline_data_t &);
};

