#pragma once

#include "glGribWorld.h"
#include "glGribOptions.h"
#include "glGribGeometry.h"
#include "glGribPalette.h"
#include "glGribFieldMetadata.h"
#include "glGribFieldFloatBuffer.h"
#include "glGribLoader.h"
#include "glGribString.h"

#include <string>
#include <algorithm>
#include <vector>


class glGribField : public glGribWorld
{
public:

  static glGribField * create (const glGribOptionsField &, float, glGribLoader *);

  typedef enum
  {
    SCALAR=0,
    CONTOUR=1,
    VECTOR=2,
    STREAM=3,
    ISOFILL=4,
  } kind;

  virtual kind getKind () const = 0;
  virtual bool useColorBar () const  = 0;

  virtual glGribField * clone () const  = 0;
  virtual void setup (glGribLoader *, const glGribOptionsField &, float = 0) = 0;
  void setPaletteOptions (const glGribOptionsPalette &);
  void setNextPalette ();
  void scalePaletteUp (float = 0.025);
  void scalePaletteDown (float = 0.025);
  void setupHilo (glgrib_field_float_buffer_ptr);
  void renderHilo (const glGribView &) const;
  void setupVertexAttributesFrame ();
  void renderFrame (const glGribView &) const;

  virtual ~glGribField () {}
  virtual std::vector<float> getValue (int index) const 
  { 
    std::vector<float> val;
    for (int i = 0; i < values.size (); i++)
      if (values[i]->data () != nullptr)
        val.push_back (values[i]->data ()[index]);
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
  void clear () override;
  void resize (const glGribView &) override {}
  const std::vector<glGribFieldMetadata> & getMeta () const
  {
    return meta;
  }
  const glGribOptionsField & getOptions () const;
  void setScale (float s) { opts.scale = s; hilo.setScaleXYZ (s); }
  float getScale () const override { return opts.scale; }
  const glGribPalette & getPalette () const;
  void toggleWireframe () 
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
 
  float getSlot () const
  {
    return slot;
  }

protected:
  float slot = 0.0f;
  template <typename T>
  void unpack (float *, const int, const float, 
               const float, const float, const T *);
  template <typename T>
  void pack (const float *, const int, const float, 
             const float, const float, T *);
  template <typename T>
  void packUnpack (const float *, float *, const int, 
		   const float, const float, const float);
  template <typename T>
  void loadHeight (glgrib_opengl_buffer_ptr, glGribLoader *);
  template <typename T>
  void bindHeight (int);
  static void getUserPref (glGribOptionsField *, glGribLoader *, int);
  glGribPalette palette = paletteColdHot;
  mutable glGribOptionsField opts;
  std::vector<glGribFieldMetadata> meta;
  std::vector<glgrib_field_float_buffer_ptr> values;
  glGribString hilo;
};

