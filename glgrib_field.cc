#include "glgrib_field.h"
#include "glgrib_program.h"
#include "glgrib_palette.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

void glgrib_field::recordPaletteOptions ()
{
  opts.palette.name = palette.name; 
  opts.palette.min  = palette.min;
  opts.palette.max  = palette.max;
}

void glgrib_field::setPaletteMinMax ()
{
  // First take command line options if possible
  if (opts.palette.max != glgrib_palette::defaultMax)
    palette.max = opts.palette.max;
  if (opts.palette.min != glgrib_palette::defaultMin)
    palette.min = opts.palette.min;
  // Reset values when not sensible
  if (palette.min >= palette.max)
    {
      palette.min = glgrib_palette::defaultMin;
      palette.max = glgrib_palette::defaultMax;
    }
  // Or, if no min/max were choosen from metadata, min/max of the field
  if (! palette.hasMax ())
    palette.max = getNormedMaxValue ();
  if (! palette.hasMin ())
    palette.min = getNormedMinValue ();
}

void glgrib_field::setPaletteOptions (const glgrib_options_palette & o) 
{ 
  opts.palette = o;
  palette = glgrib_palette::by_name (opts.palette.name);
  setPaletteMinMax ();
  recordPaletteOptions ();
}

void glgrib_field::setNextPalette ()
{
  palette = glgrib_palette::next (palette);
  setPaletteMinMax ();
  recordPaletteOptions ();
}

void glgrib_field::clear ()
{
  values.clear ();
  meta.clear ();
  glgrib_world::clear ();
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
  recordPaletteOptions ();
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
  recordPaletteOptions ();
}
