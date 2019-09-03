#ifndef _GLGRIB_CITIES_H
#define _GLGRIB_CITIES_H

#include "glgrib_points.h"
#include "glgrib_view.h"
#include "glgrib_options.h"
#include <string>
#include <vector>


class glgrib_cities : public glgrib_points
{
public:
  void init (const glgrib_options_cities &);
  glgrib_options_cities opts;
private:
};

#endif
