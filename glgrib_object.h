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
  glgrib_program * get_program () 
    { 
      if (program == NULL) 
        program = glgrib_program_load (get_program_kind ());
      return program;
    }
  virtual int get_program_kind () { return -1; }
  virtual bool use_alpha () { return true; }
  glgrib_program * program = NULL;
};

#endif
