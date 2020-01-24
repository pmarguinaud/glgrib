#ifndef _GLGRIB_FIELD_ISOFILL_H
#define _GLGRIB_FIELD_ISOFILL_H

#include "glgrib_field.h"

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
  void processTriangle1 (const float *, int, const std::vector<float> &);
  void processTriangle2 (const float [3], const glm::vec3 [3], const std::vector<float> &);

  class processTriangle2_ctx_t
  {
  public:
    int I = -1;
    glm::vec2 lonlat_J, lonlat_K;
    glm::vec2 lonlat[3];
    const float * v = NULL;
  };
  class isoband_t
  {
  public:
    glgrib_option_color color;

    int size1;
    GLuint VertexArrayID1 = 0;
    glgrib_opengl_buffer_ptr elementbuffer1;
    std::vector<unsigned int> ind1;

    int size2;
    GLuint VertexArrayID2 = 0;
    glgrib_opengl_buffer_ptr vertexbuffer2, elementbuffer2;
    std::vector<unsigned int> ind2;
    std::vector<float> lonlat2;

    void push_lonlat (const glm::vec2 & lonlat)
    {
      lonlat2.push_back (lonlat.x);
      lonlat2.push_back (lonlat.y);
    };
    void new_quad (const glm::vec2 & lonlata, 
          	   const glm::vec2 & lonlatb, 
          	   const glm::vec2 & lonlatc, 
          	   const glm::vec2 & lonlatd,
          	   bool direct)
    {
      int ind0 = lonlat2.size () / 2;

      push_lonlat (lonlata);
      push_lonlat (lonlatb);
      push_lonlat (lonlatc);
      push_lonlat (lonlatd);
      
      int ord[6] = {0, 1, 2, 0, 2, 3};

      if (! direct)
        {
          std::swap (ord[0], ord[1]);
          std::swap (ord[3], ord[4]);
        }

      for (int i = 0; i < 6; i++)
        ind2.push_back (ind0+ord[i]);
    };
    void new_tri (const glm::vec2 & lonlata, 
          	  const glm::vec2 & lonlatb, 
          	  const glm::vec2 & lonlatc)
    {
      int ind0 = lonlat2.size () / 2;

      push_lonlat (lonlata);
      push_lonlat (lonlatb);
      push_lonlat (lonlatc);

      ind2.push_back (ind0+0);
      ind2.push_back (ind0+1);
      ind2.push_back (ind0+2);
    };

    void new_penta (const glm::vec2 & lonlata, 
          	    const glm::vec2 & lonlatb, 
          	    const glm::vec2 & lonlatc,
          	    const glm::vec2 & lonlatd,
          	    const glm::vec2 & lonlate)
    {
      int ind0 = lonlat2.size () / 2;

      push_lonlat (lonlata);
      push_lonlat (lonlatb);
      push_lonlat (lonlatc);
      push_lonlat (lonlatd);
      push_lonlat (lonlate);

      ind2.push_back (ind0+0);
      ind2.push_back (ind0+1);
      ind2.push_back (ind0+2);

      ind2.push_back (ind0+0);
      ind2.push_back (ind0+2);
      ind2.push_back (ind0+3);

      ind2.push_back (ind0+0);
      ind2.push_back (ind0+3);
      ind2.push_back (ind0+4);
    };



  };
  struct
  {
    std::vector<isoband_t> isoband;
  } d;
};

#endif
