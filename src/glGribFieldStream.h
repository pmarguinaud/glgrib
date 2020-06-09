#pragma once

#include "glGribField.h"
#include <bits/stdc++.h> 

namespace glGrib
{

class FieldStream : public Field
{
public:
  explicit FieldStream (const Field::Privatizer) { }
  void setup (const Field::Privatizer, Loader *, const OptionsField &, float = 0) override;
  FieldStream (const FieldStream &) = delete;
  Field::kind getKind () const 
  {
    return Field::STREAM;
  }
  FieldStream * clone () const;
  void render (const View &, const OptionsLight &) const override;
  bool useColorBar () const override { return true; }
  int getSlotMax () const override
  {
    return (int)opts.path.size () / 2;
  }
private:
  class streamline_data_t
  {
  public:
    std::vector<float> lonlat; // Position
    std::vector<float> values; // Field value
    std::vector<float> length; // Distance 

    struct
    {
      float x = 0.0f, y = 0.0f, z = 0.0f;
    } last;

    void push (const glm::vec3 & xyz, const float d = 1.0f)
    {
      push (xyz.x, xyz.y, xyz.z, d);
    }
    void push (const float x, const float y, const float z, const float d = 1.) 
    {
      double D = 0.0f;
      float lon = 0.0f, lat = 2 * M_PI;
      if (d > 0)
        {
          lat = std::asin (z);
	  lon = atan2 (y, x);
          int sz = size (), ilast = sz - 1;
          if (sz > 0)
            {
              if ((last.x != 0.0f) || (last.y != 0.0f) || (last.z != 0.0f))
                {
                  float dx = x - last.x;
                  float dy = y - last.y;
                  float dz = z - last.z;
                  D = length[ilast] + sqrt (dx * dx + dy * dy + dz * dz) / d;
                }
            }
        }
      last.x = x;
      last.y = y;
      last.z = z;
      lonlat.push_back (lon);
      lonlat.push_back (lat);
      values.push_back (d);
      length.push_back (D);
    }
    void pop ()
    {
      lonlat.pop_back (); 
      lonlat.pop_back (); 
      values.pop_back (); 
      length.pop_back (); 
    }
    void clear ()
    {
      lonlat.clear (); 
      values.clear (); 
      length.clear (); 
    }
    int size () const
    {
      return values.size ();
    }
  };
  class streamline_t
  {
  public:
    streamline_t () : VAID (this) {}
    explicit streamline_t (const streamline_t & stream) : VAID (this)
    {
      d = stream.d;
    }
    void setupVertexAttributes () const;
    void setup (const streamline_data_t &, GLint [], GLint [], GLint []);
    void clear ()
    {
      VAID.clear ();
    }
    void render (const bool &, const float &, const glGrib::View &) const;
    struct
    {
      OpenGLBufferPtr<float> vertexbuffer, normalbuffer, distancebuffer;
      GLuint size;
      GLint vertexLonLat_attr[3];
      GLint norm_attr[2];
      GLint dist_attr[2];
    } d;
    OpenGLVertexArray<streamline_t> VAID;
  };

  class stream_seen_t : public std::set<int>
  {
  public:
    bool has (const int & k)
    {
      return end () != find (k);
    }
    bool operator[] (const int & k)
    {
      return has (k);
    }
    void add (const int & k)
    {
      insert (k);
    }
    void del (const int & k)
    {
      erase (k);
    }
  };

  void getFirstPoint (int, const BufferPtr<float> &, const BufferPtr<float> &, 
		      glm::vec2 &, glm::vec2 &, glm::vec2 &,
		      std::valarray<float> &, std::valarray<float> &, 
		      int &, int &);
  void computeStreamLine (int, const BufferPtr<float> &, const BufferPtr<float> &, streamline_data_t *);
  void computeStreamLineDir (int, const BufferPtr<float> &, const BufferPtr<float> &, 
                             const glm::vec2 &, const glm::vec2 &,
                             stream_seen_t &, float, std::valarray<float>,
                             std::vector<glm::vec3> &);

  struct
  {
    std::vector<streamline_t> stream;
    float normmax;
    double time0 = 0.0f;
  } d;

};


}
