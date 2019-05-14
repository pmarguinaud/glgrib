#ifndef _GLGRIB_FIELD_H
#define _GLGRIB_FIELD_H

#include "glgrib_world.h"
#include "glgrib_options.h"
#include "glgrib_geometry.h"
#include "glgrib_palette.h"

#include <string>
#include <memory>


class glgrib_field_float_buffer
{
public:
  glgrib_field_float_buffer (size_t);
  glgrib_field_float_buffer (float *);
  ~glgrib_field_float_buffer ();
  float * data () { return data_; }
  float & operator[] (int i) { return data_[i]; }
private:
  float * data_ = NULL;
};

typedef std::shared_ptr<glgrib_field_float_buffer> glgrib_field_float_buffer_ptr;

extern glgrib_field_float_buffer_ptr new_glgrib_field_float_buffer_ptr (size_t);
extern glgrib_field_float_buffer_ptr new_glgrib_field_float_buffer_ptr (float *);


class glgrib_field_display_options
{
public:
  float scale = 1.0;
  glgrib_palette palette;
  glgrib_field_display_options ();
};

class glgrib_field : public glgrib_world
{
public:
  glgrib_field & operator= (const glgrib_field &);
  glgrib_field () { }
  glgrib_field (const glgrib_field &);
  void init (const glgrib_options_field &, int = 0);
  virtual glgrib_program_kind get_program_kind () const 
    { 
      return GLGRIB_PROGRAM_GRADIENT_FLAT_SCALE;
    }
  virtual bool use_alpha () { return false; }
  virtual void render (const glgrib_view *) const;
  glgrib_field_float_buffer_ptr values = NULL;
  virtual float getValue (int index) const { return values != NULL ? values->data ()[index] : 9999.0f; }
  float valmis, valmin, valmax;
  virtual ~glgrib_field ();
  glgrib_field_display_options dopts;
};

#endif
