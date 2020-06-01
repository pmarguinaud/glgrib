#pragma once

#include "glGribOpenGL.h"
#include "glGribView.h"
#include "glGribOptions.h"
#include "glGribProgram.h"

namespace glGrib
{

class Object
{
public:
  bool isReady () const { return ready_; }
  void setReady () { ready_ = true; }
  virtual void hide () = 0;
  virtual void show () = 0;
  virtual bool visible () const = 0;
  virtual void reSize (const View &) = 0;
private:
  bool ready_ = false;
};

class Object2D : public Object
{
public:
  virtual void render (const glm::mat4 &) const = 0;
  bool visible () const override
  {
    return true;
  }
  void hide () override {}
  void show () override {}
};


class Object3D : public Object
{
public:
  virtual void render (const View &, const OptionsLight &) const = 0;
  bool visible () const override
  {
    return true;
  }
  void hide () override {}
  void show () override {}
  virtual float getScale () const  = 0;
};



}
