
#pragma once

#include "glGribObject.h"
#include "glGribFont.h"
#include "glGribView.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glGribOpenGL.h"
#include <string>
#include <vector>


namespace glGrib
{

template <bool SHARED, bool CHANGE>
class String2D;

template <bool SHARED, bool CHANGE>
class String3D;

namespace StringTypes
{
  typedef std::vector<float> float_v;
  typedef std::vector<std::string> string_v;

  static const int CX = 0x01;
  static const int WX = 0x00;
  static const int EX = 0x02;
  static const int CY = 0x01 << 8;
  static const int NY = 0x02 << 8;
  static const int SY = 0x00 << 8;

  typedef enum
    {
      C   = CY | CX, N   = NY | CX, S   = SY | CX, 
      W   = CY | WX, E   = CY | EX, NE  = NY | EX, 
      SE  = SY | EX, NW  = NY | WX, SW  = SY | WX,
    }
  align_t;

  typedef std::vector<align_t> align_v;

  align_t str2align (const std::string &);

};


#define ENABLE_IF_CHANGE \
template <bool OK = CHANGE> \
typename std::enable_if<OK,void>::type


template <bool SHARED, bool CHANGE>
class String 
{
public:
  typedef StringTypes::float_v  float_v;
  typedef StringTypes::string_v string_v;
  typedef StringTypes::align_t  align_t;
  typedef StringTypes::align_v  align_v;
  

  void setupVertexAttributes () const;

private:

  template <bool OK = SHARED || CHANGE> 
  typename std::enable_if<OK,String &>::type
  operator= (const String &);

  void do_copy (const String &);

  String () : VAID (this) {}
  String (const String & str) : VAID (this)
  {
    *this = str;
  }

  void setup (const_FontPtr, const string_v &, 
              const float_v &, const float_v &, 
              float = 1.0f, const align_v & = align_v{StringTypes::SW},
              const float_v & = float_v{}, const float_v & = float_v{},
              const float_v & = float_v{}, const float_v & = float_v{});


  void setForegroundColor (const OptionColor & color)
  {
    d.color0 = color;
  }
  void setBackgroundColor (const OptionColor & color)
  {
    d.color1 = color;
  }

  ENABLE_IF_CHANGE
  update (const string_v & str)
  {
    do_update (str);
  }

  ENABLE_IF_CHANGE
  update (const std::string & str)
  {
    update (string_v{str});
  }

  bool isReady () const { return d.ready; }

  void setScaleXYZ (float s)
  {
    d.scaleXYZ = s;
  }

private:
  ENABLE_IF_CHANGE
  do_update (const string_v &);

  bool _visible = true;
  const bool & getVisibleRef () const 
  {  
    return _visible;
  }

  struct 
  {
    bool shared = SHARED;
    bool change = CHANGE;
    bool ready = false;
    string_v data;
    float_v x, y;       // Position of letters vertices
    float_v X, Y, Z, A; // Position & angle of each letter on the sphere
    align_v align;
    OptionColor color0 = OptionColor (255, 255, 255, 255);
    OptionColor color1 = OptionColor (  0,   0,   0,   0);
    float scale;
    float scaleXYZ = 1.0f;
    int len;    // Total number of characters
    OpenGLBufferPtr<float> xyzbuffer, vertexbuffer, letterbuffer;
    const_FontPtr font;
  } d;
  OpenGLVertexArray<String> VAID;
  friend class String2D<SHARED,CHANGE>;
  friend class String3D<SHARED,CHANGE>;
};


template <bool SHARED, bool CHANGE>
class String2D : public Object2D
{
public:
  typedef StringTypes::float_v  float_v;
  typedef StringTypes::string_v string_v;
  typedef StringTypes::align_t  align_t;
  typedef StringTypes::align_v  align_v;

  void render (const glm::mat4 &) const override;

  void setup (glGrib::const_FontPtr ff, const string_v & str, 
              float x, float y, float s = 1.0f, align_t align = StringTypes::SW)
  {
    this->str.setup (ff, str, float_v{x}, float_v{y}, s, align_v{align});
    setReady ();
  }
  
  void setup (glGrib::const_FontPtr ff, const string_v & str, 
              const float_v & x = float_v{}, const float_v & y = float_v{}, 
              float s = 1.0f, align_t align = StringTypes::SW, 
              const float_v & a = float_v{})
  {
    this->str.setup (ff, str, x, y, s, align_v{align},
           float_v{}, float_v{}, float_v{}, a);
    setReady ();
  }
  
  void setup (glGrib::const_FontPtr ff, const string_v & str, 
              const float_v & x, const float_v & y, 
              float s, const align_v & align,
              const float_v & a)
  {
    this->str.setup (ff, str, x, y, s, align, 
           float_v{}, float_v{}, float_v{}, a);
    setReady ();
  }
  
  void setup (glGrib::const_FontPtr ff, const std::string & str, 
              float x, float y, float s = 1.0f, align_t align = StringTypes::SW)
  {
    string_v _str = {str};
    float_v       _x   = {x};
    float_v       _y   = {y};
    this->str.setup (ff, _str, _x, _y, s, align_v{align});
    setReady ();
  }

  void setForegroundColor (const OptionColor & color)
  {
    str.setForegroundColor (color);
  }

  void setBackgroundColor (const OptionColor & color)
  {
    str.setBackgroundColor (color);
  }

  ENABLE_IF_CHANGE
  update (const string_v & str)
  {
    this->str.update (str);
  }

  ENABLE_IF_CHANGE
  update (const std::string & str)
  {
    this->str.update (str);
  }

  void reSize (const View &) override {}

  Object2D::side_t getSide () const override { return side; }
  void setSide (Object2D::side_t s) { side = s; }

private:
  Object2D::side_t side = Object2D::LEFT;
  String<SHARED,CHANGE> str;
};

template <bool SHARED, bool CHANGE>
class String3D : public Object3D
{
public:
  typedef StringTypes::float_v  float_v;
  typedef StringTypes::string_v string_v;
  typedef StringTypes::align_t  align_t;
  typedef StringTypes::align_v  align_v;

  void render (const View &, const OptionsLight &) const;

  void setup (glGrib::const_FontPtr ff, const string_v & str, 
              const float_v & _X = float_v{}, const float_v & _Y = float_v{},
              const float_v & _Z = float_v{}, const float_v & _A = float_v{},
              float s = 1.0f, align_t _align = StringTypes::SW)
  {
    float_v _x, _y;
    for (size_t i = 0; i < str.size (); i++)
      {
        _x.push_back (0.0f);
        _y.push_back (0.0f);
      }
    this->str.setup (ff, str, _x, _y, s, align_v{_align}, _X, _Y, _Z, _A);
    setReady ();
  }

  void setForegroundColor (const OptionColor & color)
  {
    str.setForegroundColor (color);
  }

  void setBackgroundColor (const OptionColor & color)
  {
    str.setBackgroundColor (color);
  }

  ENABLE_IF_CHANGE
  update (const string_v & str)
  {
    this->str.update (str);
  }

  ENABLE_IF_CHANGE
  update (const std::string & str)
  {
    this->str.update (str);
  }

  float getScale () const override
  {
    return str.d.scaleXYZ;
  }

  void reSize (const View &) override {}

  void setScale (float s) 
  {
    str.setScaleXYZ (s);
  }

  void show () override {}
  void hide () override {}

private:

  const bool & getVisibleRef () const override
  {  
    return str.getVisibleRef ();
  }

  String<SHARED,CHANGE> str;
};

}
