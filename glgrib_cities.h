#ifndef _GLGRIB_CITIES_H
#define _GLGRIB_CITIES_H

#include "glgrib_points.h"
#include "glgrib_view.h"
#include "glgrib_options.h"
#include "glgrib_string.h"
#include <string>
#include <vector>


class glgrib_cities : public glgrib_points
{
public:
  void render (const glgrib_view &) const;
  glgrib_cities & operator= (const glgrib_cities &);
  void clear ();
  void setup (const glgrib_options_cities &);
  glgrib_options_cities opts;
private:
  struct
  {
    glgrib_string labels;
  } d;
};

#endif
