#include "glgrib_field.h"
#include "glgrib_load.h"
#include "glgrib_program.h"
#include "glgrib_palette.h"

#include <stdlib.h>
#include <stdio.h>

void glgrib_field::init (const std::string & field, const glgrib_options & o, const glgrib_geometry * geom)
{
  unsigned char * col;

  geometry = geom;

  ncol = 1;

  glgrib_load (field, &values, &valmin, &valmax, &valmis);

  col = (unsigned char *)malloc (ncol * geom->np * sizeof (unsigned char));

  for (int i = 0; i < geom->np; i++)
    if (values[i] == valmis)
      col[i] = 0;
    else
      col[i] = 1 + (int)(254 * (values[i] - valmin)/(valmax - valmin));

  def_from_vertexbuffer_col_elementbuffer (col, geom);

  free (col);
}

glgrib_field_display_options::glgrib_field_display_options ()
{
    palette = palette_cloud_auto;
    palette = palette_cold_hot;
}

void glgrib_field::render (const glgrib_view * view, const glgrib_field_display_options & dopts) const
{
  const glgrib_program * program = get_program (); 
  float scale0[3] = {dopts.scale, dopts.scale, dopts.scale};
  const glgrib_palette & p = dopts.palette;

  p.setRGBA255 (program->programID);


  glUniform3fv (glGetUniformLocation (program->programID, "scale0"), 1, scale0);
  glUniform1f (glGetUniformLocation (program->programID, "valmin"), valmin);
  glUniform1f (glGetUniformLocation (program->programID, "valmax"), valmax);

  float palmax = p.hasMax () ? p.getMax () : valmax;
  float palmin = p.hasMin () ? p.getMin () : valmin;

  glUniform1f (glGetUniformLocation (program->programID, "palmin"), palmin);
  glUniform1f (glGetUniformLocation (program->programID, "palmax"), palmax);

  glgrib_world::render (view);
}

glgrib_field::~glgrib_field ()
{
  if (values)
    free (values);
}


