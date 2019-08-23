#include "glgrib_field.h"
#include "glgrib_program.h"
#include "glgrib_palette.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

void glgrib_field::recordPaletteOpts ()
{
  opts.palette.name = palette.name; 
  opts.palette.min  = palette.min;
  opts.palette.max  = palette.max;
}

void glgrib_field::setPaletteMinMax ()
{
  if (! palette.hasMax ())
    palette.max = opts.palette.max;
  if (! palette.hasMin ())
    palette.min = opts.palette.min;
}

void glgrib_field::setPalette (const glgrib_palette & p) 
{ 
  palette = p; 
  setPaletteMinMax ();
  recordPaletteOpts ();
}

void glgrib_field::setNextPalette ()
{
  palette = glgrib_palette::get_next (palette);
  setPaletteMinMax ();
  recordPaletteOpts ();
}

void glgrib_field::cleanup ()
{
  values.clear ();
  meta.clear ();
  glgrib_world::cleanup ();
}

void glgrib_field::scalePaletteUp (float x)
{
  if (! palette.hasMin ()) 
    palette.min = getNormedMinValue (); 
  if (! palette.hasMax ()) 
    palette.max = getNormedMaxValue (); 
  float d = palette.max - palette.min;
  palette.min -= d * x;
  palette.max += d * x;
  recordPaletteOpts ();
}

void glgrib_field::scalePaletteDown (float x)
{
  if (! palette.hasMin ()) 
    palette.min = getNormedMinValue (); 
  if (! palette.hasMax ()) 
    palette.max = getNormedMaxValue (); 
  float d = palette.max - palette.min;
  palette.min += d * x;
  palette.max -= d * x;
  recordPaletteOpts ();
}
