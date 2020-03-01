#pragma once

#include "glGribField.h"
#include "glGribString.h"

class glGribFieldContour : public glGribField
{
public:
  glGribField::kind getKind () const 
  {
    return glGribField::CONTOUR;
  }
  glGribFieldContour * clone () const;
  glGribFieldContour & operator= (const glGribFieldContour &);
  glGribFieldContour () { }
  glGribFieldContour (const glGribFieldContour &);
  void setup (glGribLoader *, const glGribOptionsField &, float = 0) override;
  void render (const glGribView &, const glGribOptionsLight &) const override;
  virtual ~glGribFieldContour ();
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
    glGribString labels;
    float level;
    bool wide = false;
    float width = 0.0f;
    bool dash = false;
    float length = 0.0f;
    glGribOptionColor color;
    std::vector<int> pattern;
    GLuint VertexArrayID;
    glgrib_opengl_buffer_ptr vertexbuffer, heightbuffer, distancebuffer;
    GLuint size;
  };

  std::vector<isoline_t> iso;

  void processTriangle (int, float *, float, float *, float, float, float, bool *, isoline_data_t *);
  void setupLabels (isoline_t *, const isoline_data_t &);
};

