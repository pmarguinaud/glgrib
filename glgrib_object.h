#ifndef _GLGRIB_OBJECT_H
#define _GLGRIB_OBJECT_H

#include "glgrib_opengl.h"
#include "glgrib_view.h"
#include "glgrib_program.h"

class glgrib_object
{
public:
  virtual void render (const glgrib_view *) const = 0;
  void load_program ();
  glgrib_program * get_program () const
    { 
      if (program == NULL) 
        program = glgrib_program_load (get_program_kind ());
      return program;
    }
  virtual glgrib_program_kind get_program_kind () const { return GLGRIB_PROGRAM_NONE; }
  virtual bool use_alpha () { return true; }
  mutable glgrib_program * program = NULL;
  bool isReady () const { return ready_; }
  void setReady () { ready_ = true; }
private:
  bool ready_ = false;
};

#endif
