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
  virtual void render (const View &, const OptionsLight &) const = 0;
  bool isReady () const { return ready_; }
  void setReady () { ready_ = true; }
  void hide () { hidden = true; }
  void show () { hidden = false; }
  bool visible () const { return ! hidden; }
  virtual void reSize (const View &) = 0;
  virtual float getScale () const  = 0;
private:
  bool ready_ = false;
  bool hidden = false;
};



}
