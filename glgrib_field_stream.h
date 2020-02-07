#pragma once

#include "glgrib_field.h"
#include <bits/stdc++.h> 

class glgrib_field_stream : public glgrib_field
{
public:
  glgrib_field::kind getKind () const 
  {
    return glgrib_field::STREAM;
  }
  glgrib_field_stream * clone () const;
  glgrib_field_stream & operator= (const glgrib_field_stream &);
  glgrib_field_stream () { }
  glgrib_field_stream (const glgrib_field_stream &);
  void setup (glgrib_loader *, const glgrib_options_field &, float = 0) override;
  void render (const glgrib_view &, const glgrib_options_light &) const override;
  virtual ~glgrib_field_stream ();
  void setupVertexAttributes ();
  void clear () override;
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
          lat = asin (z);
	  lon = atan2 (y, x);
          int sz = size (), ilast = sz - 1;
          if (sz > 0)
            {
              if ((last.x != 0.0f) || (last.y != 0.0f) || (last.z != 0.0f))
                {
                  float dx = x - last.x;
                  float dy = y - last.y;
                  float dz = z - last.z;
                  D = length[ilast] + sqrt (dx * dx + dy * dy + dz * dz);
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
    int size ()
    {
      return values.size ();
    }
  };
  class streamline_t
  {
  public:
    GLuint VertexArrayID;
    glgrib_opengl_buffer_ptr vertexbuffer, normalbuffer, distancebuffer;
    GLuint size;
  };

  std::vector<streamline_t> stream;
  float normmax;

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

  void getFirstPoint (int, const float *, const float *, 
		      glm::vec2 &, glm::vec2 &, glm::vec2 &,
		      std::valarray<float> &, std::valarray<float> &, 
		      int &, int &);
  void computeStreamLine (int, const float *, const float *, streamline_data_t *);
  void computeStreamLineDir (int, const float *, const float *, 
                             const glm::vec2 &, const glm::vec2 &,
                             stream_seen_t &, float, std::valarray<float>,
                             std::vector<glm::vec3> &);
};

