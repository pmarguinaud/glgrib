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

  };
  struct
  {
    std::vector<isoband_t> isoband;
  } d;
};

#endif
