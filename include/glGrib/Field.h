#pragma once

#include "glGrib/World.h"
#include "glGrib/Options.h"
#include "glGrib/Geometry.h"
#include "glGrib/Palette.h"
#include "glGrib/FieldMetadata.h"
#include "glGrib/Buffer.h"
#include "glGrib/Loader.h"
#include "glGrib/String.h"

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


template <int N>
class FieldPacked;

template <int N>
class FieldScalar;

class Field : public World
{
public:

  static Field * create (const OptionsField &, Loader *);
  Field (const Field &) = delete;

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

  virtual Field * clone () const  = 0;
  void setPaletteOptions (const OptionsPalette & o)
  {
    palette = glGrib::Palette (o, getNormedMinValue (), getNormedMaxValue ());
  }

  const std::vector<BufferPtr<float>> & getValues () const
  {
    return values;
  }

  virtual const std::vector<float> getValue (int index) const 
  { 
    std::vector<float> val;
    for (size_t i = 0; i < values.size (); i++)
      if (values[i].allocated ())
        val.push_back (values[i][index]);
    return val;
  }
  virtual const std::vector<float> getMaxValue () const 
  { 
    std::vector<float> val; 
    for (size_t i = 0; i < meta.size (); i++) val.push_back (meta[i].valmax); 
    return val; 
  }
  virtual const std::vector<float> getMinValue () const 
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
    return std::sqrt (n);
  }
  virtual float getNormedMaxValue () const 
  {
    std::vector<float> val = getMaxValue ();
    if (val.size () == 1)
      return val[0];
    float n = 0.0f;
    for (size_t i = 0; i < val.size (); i++)
      n += val[i] * val[i];
    return std::sqrt (n);
  }
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
  void setScale (float s) { opts.scale = s; hilo.setScale (s); }
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


  virtual int getSlotMax () const  = 0;
 
  float getSlot () const
  {
    return opts.slot;
  }

  void saveOptions () const;

  virtual void update ();

  virtual int getNearestPoint (float lat, float lon) const
  {
    return getGeometry ()->latlon2index (lat, lon);
  }

private:

  class Privatizer
  {
  };

  static void getUserPref (OptionsField *, Loader *, int);

  virtual void setup (const Field::Privatizer, Loader *, const OptionsField &) = 0;
  void setupHilo (BufferPtr<float>);
  void renderHilo (const View &) const;
  void renderFrame (const View & view) const 
  {
    frame.VAID.render (view);
  }

  Field () : frame (this) {}

  const bool & getVisibleRef () const override
  {  
    return opts.visible.on;
  }

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
      field->getGeometry ()->bindFrame (0);
    }
    Field * field = nullptr;
    OpenGLVertexArray<frame_t> VAID;
  };

  String3D<0,1> hilo;
  frame_t frame;

private:
  int frameNumber = 0;
  Palette palette;
  mutable OptionsField opts;
  std::vector<FieldMetadata> meta;
  std::vector<BufferPtr<float>> values;
  friend class FieldPacked< 8>;
  friend class FieldPacked<16>;
  friend class FieldPacked<32>;
  friend class FieldVector;
  friend class FieldScalar< 8>;
  friend class FieldScalar<16>;
  friend class FieldScalar<32>;
  friend class FieldIsoFill;
  friend class FieldStream;
  friend class FieldContour;
};

template <int N>
class FieldPacked : public Field
{
public:

  using T = typename FieldPackingType<N>::type;
  FieldPacked () : Field () {}
  FieldPacked (const FieldPacked &) = delete;

private:
  void unpack (BufferPtr<float> &, const int, const float, 
               const float, const float, const OpenGLBufferPtr<T> &);
  void pack (const BufferPtr<float> &, const int, const float, 
             const float, const float, OpenGLBufferPtr<T> &);
  void packUnpack (const BufferPtr<float> &, BufferPtr<float> &, const int, 
		   const float, const float, const float);
  void loadHeight (OpenGLBufferPtr<T>, Loader *);
  void createMask (glGrib::Loader *);
  void bindHeight (int) const;
  void bindMask (int) const;
  OpenGLBufferPtr<T> heightbuffer;
  OpenGLBufferPtr<float> maskbuffer;
  friend class FieldVector;
  friend class FieldScalar< 8>;
  friend class FieldScalar<16>;
  friend class FieldScalar<32>;
  friend class FieldIsoFill;
  friend class FieldStream;
  friend class FieldContour;
};


}
