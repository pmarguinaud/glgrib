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


namespace glGrib
{


template <int N>
struct FieldPackingType
{
};

template <> struct FieldPackingType<8>
{
  typedef unsigned char type;
};

template <> struct FieldPackingType<16>
{
  typedef unsigned short type;
};

template <> struct FieldPackingType<32>
{
  typedef unsigned int type;
};


class Field : public World
{
public:

  static Field * create (const OptionsField &, float, Loader *);

  typedef enum
  {
    SCALAR=0,
    CONTOUR=1,
    VECTOR=2,
    STREAM=3,
    ISOFILL=4,
  } kind;

  Field () : frame (this) {}

  virtual kind getKind () const = 0;
  virtual bool useColorBar () const  = 0;

  virtual Field * clone () const  = 0;
  virtual void setup (Loader *, const OptionsField &, float = 0) = 0;
  void setPaletteOptions (const OptionsPalette & o)
  {
    palette = glGrib::Palette (o, getNormedMinValue (), getNormedMaxValue ());
  }
  void setupHilo (FieldFloatBufferPtr);
  void renderHilo (const View &) const;
  void renderFrame (const View & view) const 
  {
    frame.VAID.render (view);
  }
  virtual std::vector<float> getValue (int index) const 
  { 
    std::vector<float> val;
    for (size_t i = 0; i < values.size (); i++)
      if (values[i]->data () != nullptr)
        val.push_back (values[i]->data ()[index]);
    return val;
  }
  virtual std::vector<float> getMaxValue () const 
  { 
    std::vector<float> val; 
    for (size_t i = 0; i < meta.size (); i++) val.push_back (meta[i].valmax); 
    return val; 
  }
  virtual std::vector<float> getMinValue () const 
  { 
    std::vector<float> val; 
    for (size_t i = 0; i < meta.size (); i++) val.push_back (meta[i].valmin); 
    return val; 
  }
  virtual float getNormedMinValue () const 
  {
    std::vector<float> val = getMinValue ();
    if (val.size () == 1)
      return val[0];
    float n = 0.0f;
    for (size_t i = 0; i < val.size (); i++)
      n += val[i] * val[i];
    return sqrt (n);
  }
  virtual float getNormedMaxValue () const 
  {
    std::vector<float> val = getMaxValue ();
    if (val.size () == 1)
      return val[0];
    float n = 0.0f;
    for (size_t i = 0; i < val.size (); i++)
      n += val[i] * val[i];
    return sqrt (n);
  }
  void clear () override;
  void reSize (const View &) override {}
  const std::vector<FieldMetadata> & getMeta () const
  {
    return meta;
  }
  const OptionsField & getOptions () const
  {
    opts.palette = palette.getOptions ();
    return opts; 
  }
  void setScale (float s) { opts.scale = s; hilo.setScaleXYZ (s); }
  float getScale () const override { return opts.scale; }
  const Palette & getPalette () const
  {
    return palette;
  }
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

  static void getUserPref (OptionsField *, Loader *, int);

  class frame_t
  {
  public:
    frame_t (Field * f) : field (f), VAID (this) {}
    frame_t & operator= (const frame_t & s)
    {   
      if (this != &s) 
        VAID = s.VAID;
      return *this;
    }   
    void clear ()
    {
      VAID.clear ();
    }
    void render (const View &) const;
    void setupVertexAttributes () const
    {
      field->geometry->bindFrame (0);
    }
    Field * field = nullptr;
    OpenGLVertexArray<frame_t> VAID;
  };

  float slot = 0.0f;
  Palette palette;
  mutable OptionsField opts;
  std::vector<FieldMetadata> meta;
  std::vector<FieldFloatBufferPtr> values;
  String hilo;
  frame_t frame;
};

template <int N>
class FieldPacked : public Field
{
public:

  using T = typename FieldPackingType<N>::type;

  FieldPacked () : Field () {}

protected:
  void unpack (float *, const int, const float, 
               const float, const float, const T *);
  void pack (const float *, const int, const float, 
             const float, const float, T *);
  void packUnpack (const float *, float *, const int, 
		   const float, const float, const float);
  void loadHeight (OpenGLBufferPtr, Loader *);
  void bindHeight (int) const;
  OpenGLBufferPtr heightbuffer;
};


}
