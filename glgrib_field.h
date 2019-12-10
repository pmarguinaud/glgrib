#ifndef _GLGRIB_FIELD_H
#define _GLGRIB_FIELD_H

#include "glgrib_world.h"
#include "glgrib_options.h"
#include "glgrib_geometry.h"
#include "glgrib_palette.h"
#include "glgrib_field_metadata.h"
#include "glgrib_field_float_buffer.h"
#include "glgrib_loader.h"
#include "glgrib_string.h"

#include <string>
#include <algorithm>
#include <vector>


class glgrib_field : public glgrib_world
{
public:

  static glgrib_field * create (const glgrib_options_field &, float, glgrib_loader *);

  typedef enum
  {
    SCALAR=0,
    CONTOUR=1,
    VECTOR=2,
    STREAM=3,
  } kind;

  virtual kind getKind () const = 0;
  virtual bool useColorBar () const  = 0;

  virtual glgrib_field * clone () const  = 0;
  virtual void setup (glgrib_loader *, const glgrib_options_field &, float = 0) = 0;
  virtual bool use_alpha () { return false; }
  void setPaletteOptions (const glgrib_options_palette &);
  void setNextPalette ();
  void scalePaletteUp (float = 0.025);
  void scalePaletteDown (float = 0.025);
  void setupHilo (glgrib_field_float_buffer_ptr);
  void renderHilo (const glgrib_view &) const;

  virtual ~glgrib_field () {}
  virtual std::vector<float> getValue (int index) const 
  { 
    std::vector<float> val;
    for (int i = 0; i < values.size (); i++)
      val.push_back (values[i] != NULL ? values[i]->data ()[index] : 9999.0f);
    return val;
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
  virtual void clear ();
  virtual void resize (const glgrib_view &) {}
  const std::vector<glgrib_field_metadata> & getMeta () const
  {
    return meta;
  }
  const glgrib_options_field & getOptions () const;
  void setScale (float s) { opts.scale = s; hilo.setScaleXYZ (s); }
  const glgrib_palette & getPalette () const;
  void toggle_wireframe () 
  {
    if (opts.scalar.wireframe.on)
      {
        opts.scalar.wireframe.on = false;
        opts.scalar.points.on = true;
      }
    else if (opts.scalar.points.on)
      {
        opts.scalar.wireframe.on = false;
        opts.scalar.points.on = false;
      }
    else
      {
        opts.scalar.wireframe.on = true;
        opts.scalar.points.on = false;
      }
 
  }

  void saveOptions () const;

  virtual int getSlotMax () const  = 0;


protected:
  virtual void loadHeight (glgrib_opengl_buffer_ptr, glgrib_loader *);
  virtual void bindHeight (int);
  static void getUserPref (glgrib_options_field *, glgrib_loader *);
  glgrib_palette palette = palette_cold_hot;
  mutable glgrib_options_field opts;
  std::vector<glgrib_field_metadata> meta;
  std::vector<glgrib_field_float_buffer_ptr> values;
  glgrib_string hilo;
};

#endif
