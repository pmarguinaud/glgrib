#pragma once

#include "glGribOpenGL.h"
#include "glGribView.h"
#include "glGribOptions.h"
#include "glGribProgram.h"

class glGribObject
{
public:
  virtual void render (const glGribView &, const glGribOptionsLight &) const = 0;
  bool isReady () const { return ready_; }
  void setReady () { ready_ = true; }
  void hide () { hidden = true; }
  void show () { hidden = false; }
  bool visible () const { return ! hidden; }
  void clear () { ready_ = false; }
  virtual void resize (const glGribView &) = 0;
  virtual float getScale () const  = 0;
private:
  bool ready_ = false;
  bool hidden = false;
};


