#ifndef _GLGRIB_FIELD_H
#define _GLGRIB_FIELD_H

#include "glgrib_world.h"
#include "glgrib_options.h"
#include "glgrib_geometry.h"
#include "glgrib_palette.h"
#include "glgrib_field_metadata.h"
#include "glgrib_field_float_buffer.h"

#include <string>
#include <vector>



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
  virtual glgrib_field * clone () const  = 0;
  virtual void init (const glgrib_options_field &, float = 0) = 0;
  virtual bool use_alpha () { return false; }
  void setPalette (const std::string & p) { dopts.palette = get_palette_by_name (p); }
  virtual ~glgrib_field () {}
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
    if (val.size () == 1)
      return val[0];
    float n = 0.0f;
    for (int i = 0; i < val.size (); i++)
      n += val[i] * val[i];
    return sqrt (n);
  }
  virtual float getNormedMaxValue () const 
  {
    std::vector<float> val = getMaxValue ();
    if (val.size () == 1)
      return val[0];
    float n = 0.0f;
    for (int i = 0; i < val.size (); i++)
      n += val[i] * val[i];
    return sqrt (n);
  }
  virtual void cleanup ();
  virtual void resize (const glgrib_view &) {}
  const std::vector<glgrib_field_metadata> & getMeta () const
  {
    return meta;
  }
protected:
  glgrib_options_field opts;
  std::vector<glgrib_field_metadata> meta;
  std::vector<glgrib_field_float_buffer_ptr> values;
};

#endif
