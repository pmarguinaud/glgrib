#pragma once

#include "glGribField.h"
#include "glGribString.h"

namespace glGrib
{

class FieldContour : public Field
{
public:
  FieldContour (const Field::Privatizer) { }
  void setup (const Field::Privatizer, Loader *, const OptionsField &, float = 0) override;
  Field::kind getKind () const 
  {
    return Field::CONTOUR;
  }
  FieldContour * clone () const;
  void render (const View &, const OptionsLight &) const override;
  bool useColorBar () const override { return true; }
  int getSlotMax () const override
  {
    return (int)opts.path.size ();
  }
  FieldContour (const FieldContour &) = delete;
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
    isoline_t () : VAID (this) {}
    isoline_t (const isoline_t & iso) : VAID (this) 
    {
      d = iso.d;
    }
    void setup (const OptionsField &, float, size_t, const Palette &, const isoline_data_t &);
    void setupVertexAttributes () const;
    void setupLabels (const OptionsField & opts, const isoline_data_t &);
    void clear ()
    {
      VAID.clear ();
    }
    void render (const glGrib::View & view, const glGrib::OptionsLight & light) const;
    struct
    {
      String3D labels;
      float level;
      bool wide = false;
      float width = 0.0f;
      bool dash = false;
      float length = 0.0f;
      OptionColor color;
      std::vector<int> pattern;
      OpenGLBufferPtr<float> vertexbuffer, heightbuffer, distancebuffer;
      GLuint size;
    } d;
    OpenGLVertexArray<isoline_t> VAID;
  };

  std::vector<isoline_t> iso;

  void processTriangle (int, const BufferPtr<float> &, float, 
                        const BufferPtr<float> &, float, float, float, 
                        bool *, isoline_data_t *);
};


}
