#ifndef _GLGRIB_OBJECT_H
#define _GLGRIB_OBJECT_H

#include "glgrib_opengl.h"
#include "glgrib_view.h"
#include "glgrib_options.h"
#include "glgrib_program.h"

class glgrib_object
{
public:
  virtual void render (const glgrib_view &, const glgrib_options_light &) const = 0;
  bool isReady () const { return ready_; }
  void setReady () { ready_ = true; }
  void hide () { hidden = true; }
  void show () { hidden = false; }
  bool visible () const { return ! hidden; }
  void clear () { ready_ = false; }
  virtual void resize (const glgrib_view &) = 0;
  virtual float getScale () const { return 1; }
private:
  bool ready_ = false;
  bool hidden = false;
};


#endif
