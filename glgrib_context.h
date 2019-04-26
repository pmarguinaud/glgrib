#ifndef _GLGRIB_CONTEXT_H
#define _GLGRIB_CONTEXT_H

#include "glgrib_view.h"
#include "glgrib_scene.h"

class glgrib_context
{
public:
  bool do_rotate = false;
  glgrib_view * view;
  int width, height;
  int snapshot_cnt = 0;
  glgrib_scene * scene;
  bool cursorpos = false;
  const char * title = "";
};

#endif
