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
  virtual void reSize (const View &) = 0;
private:
  bool ready_ = false;
};

class Object2D : public Object
{
public:
  virtual void render (const glm::mat4 &) const = 0;
};

class Object3D : public Object
{
public:
  virtual void render (const View &, const OptionsLight &) const = 0;
  virtual float getScale () const  = 0;
  virtual void hide () 
  {
    getVisibleRef () = false;
  }
  virtual void show () 
  {
    getVisibleRef () = true;
  }
  bool visible () const
  {
    return getVisibleRef ();
  }
protected:
  virtual const bool & getVisibleRef () const = 0;
  bool & getVisibleRef () 
  {
    return const_cast<bool &>(static_cast<const Object3D &>(*this).getVisibleRef ());
  }
};



}
