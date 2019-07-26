#include "glgrib_field.h"
#include "glgrib_program.h"
#include "glgrib_palette.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

glgrib_field_display_options::glgrib_field_display_options ()
{
  palette = palette_cloud_auto;
  palette = palette_cold_hot;
}

void glgrib_field::cleanup ()
{
  values.clear ();
  meta.clear ();
  glgrib_world::cleanup ();
}
