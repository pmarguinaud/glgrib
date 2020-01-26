#ifndef _GLGRIB_FIELD_ISOFILL_H
#define _GLGRIB_FIELD_ISOFILL_H

#include "glgrib_field.h"
#include <omp.h>

class glgrib_field_isofill : public glgrib_field
{
public:
  glgrib_field::kind getKind () const 
  {
    return glgrib_field::ISOFILL;
  }
  glgrib_field_isofill * clone () const;
  glgrib_field_isofill & operator= (const glgrib_field_isofill &);
  glgrib_field_isofill () { }
  glgrib_field_isofill (const glgrib_field_isofill &);
  virtual void setup (glgrib_loader *, const glgrib_options_field &, float = 0);
  virtual void render (const glgrib_view &, const glgrib_options_light &) const;
  virtual ~glgrib_field_isofill ();
  void setupVertexAttributes ();
  virtual void clear (); 
  virtual bool useColorBar () const { return true; }
  virtual int getSlotMax () const  
  {
    return (int)opts.path.size ();
  }
private:

  class processTriangle2_ctx_t
  {
  public:
    int I = -1;
    glm::vec2 lonlat_J, lonlat_K;
    glm::vec2 lonlat[3];
    const float * v = NULL;
  };

  class isoband_maker_t
  {
  public:

    std::vector<unsigned int> ind2;
    std::vector<float> lonlat2;
    int color_index;

    void push_lonlat (const glm::vec2 & lonlat)
    {
      lonlat2.push_back (lonlat.x);
      lonlat2.push_back (lonlat.y);
    }

    template <typename T, typename... Args>
    void push_lonlat (const T & lonlat, Args... args)
    {
      push_lonlat (lonlat);
      push_lonlat (args...);
    }

    void push_indice (int ind)
    {
      ind2.push_back (ind);
    }

    template <typename T, typename... Args>
    void push_indice (T ind, Args... args)
    {
      push_indice (ind);
      push_indice (args...);
    }

    void quad (const glm::vec2 & lonlata, const glm::vec2 & lonlatb, 
               const glm::vec2 & lonlatc, const glm::vec2 & lonlatd,
               bool direct)
    {
      int ind0 = lonlat2.size () / 2;

      push_lonlat (lonlata, lonlatb, 
                   lonlatc, lonlatd);
      
      int ord[6] = {0, 1, 2, 0, 2, 3};

      if (! direct)
        {
          std::swap (ord[0], ord[1]);
          std::swap (ord[3], ord[4]);
        }

      push_indice (ind0+ord[0], ind0+ord[1], 
                   ind0+ord[2], ind0+ord[3], 
                   ind0+ord[4], ind0+ord[5]);

    }

    void tri (const glm::vec2 & lonlata, 
              const glm::vec2 & lonlatb, 
              const glm::vec2 & lonlatc)
    {
      int ind0 = lonlat2.size () / 2;

      push_lonlat (lonlata, lonlatb, lonlatc);
      push_indice (ind0+0, ind0+1, ind0+2);
    }

    void penta (const glm::vec2 & lonlata, const glm::vec2 & lonlatb, 
          	const glm::vec2 & lonlatc, const glm::vec2 & lonlatd,
          	const glm::vec2 & lonlate)
    {
      int ind0 = lonlat2.size () / 2;

      push_lonlat (lonlata, lonlatb, lonlatc,
                   lonlatd, lonlate);

      push_indice (ind0+0, ind0+1, ind0+2,
                   ind0+0, ind0+2, ind0+3,
                   ind0+0, ind0+3, ind0+4);
    }
   
  };

  class isoband_t
  {
  public:
    glgrib_option_color color;
    GLuint VertexArrayID2 = 0;
    glgrib_opengl_buffer_ptr vertexbuffer2, elementbuffer2;
    int size2;
  };

  struct
  {
    GLuint VertexArrayID1 = 0;
    glgrib_opengl_buffer_ptr colorbuffer;

    std::vector<isoband_t> isoband;
  } d;

  void processTriangle1 (std::vector<isoband_maker_t> *, const float *, 
                         int, const std::vector<float> &);
  void processTriangle2 (std::vector<isoband_maker_t> *, const float [3], 
                         const glm::vec3 [3], const std::vector<float> &, bool);

};

#endif
