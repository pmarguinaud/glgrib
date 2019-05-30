#ifndef _GLGRIB_FIELD_H
#define _GLGRIB_FIELD_H

#include "glgrib_world.h"
#include "glgrib_options.h"
#include "glgrib_geometry.h"
#include "glgrib_palette.h"
#include "glgrib_field_metadata.h"

#include <string>
#include <memory>
#include <vector>


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
  void setPalette (const std::string & p) { dopts.palette = get_palette_by_name (p); }
  virtual ~glgrib_field ();
  glgrib_field_display_options dopts;
  virtual std::vector<float> getValue (int index) const 
  { 
    std::vector<float> val;
    for (int i = 0; i < values.size (); i++)
      val.push_back (values[i] != NULL ? values[i]->data ()[index] : 9999.0f);
    return val;
  }
  virtual void saveSettings () const 
  { 
    for (int i = 0; i < meta.size (); i++)
      dopts.palette.save (meta[i]); 
  }
  virtual std::vector<float> getMaxValue () const 
  { 
    std::vector<float> val; 
    for (int i = 0; i < meta.size (); i++) val.push_back (meta[i].valmax); 
    return val; 
  }
  virtual std::vector<float> getMinValue () const 
  { 
    std::vector<float> val; 
    for (int i = 0; i < meta.size (); i++) val.push_back (meta[i].valmin); 
    return val; 
  }
  virtual float getNormedMinValue () const 
  {
    std::vector<float> val = getMinValue ();
    float n = 0.0f;
    for (int i = 0; i < val.size (); i++)
      n += val[i] * val[i];
    return sqrt (n);
  }
  virtual float getNormedMaxValue () const 
  {
    std::vector<float> val = getMaxValue ();
    float n = 0.0f;
    for (int i = 0; i < val.size (); i++)
      n += val[i] * val[i];
    return sqrt (n);
  }
private:
  std::vector<glgrib_field_metadata> meta;
  std::vector<glgrib_field_float_buffer_ptr> values;
};

#endif
