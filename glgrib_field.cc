#include "glgrib_field.h"
#include "glgrib_program.h"
#include "glgrib_palette.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

void glgrib_field::setPaletteOptions (const glgrib_options_palette & o) 
{ 
  palette = glgrib_palette::create (o, getNormedMinValue (), getNormedMaxValue ());
}

void glgrib_field::setNextPalette ()
{
  palette = glgrib_palette::next (palette, getNormedMinValue (), getNormedMaxValue ());
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
    palette.setMin (getNormedMinValue ());
  if (! palette.hasMax ()) 
    palette.setMax (getNormedMaxValue ()); 
  float d = palette.getMax () - palette.getMin ();
  palette.setMin (palette.getMin () - d * x);
  palette.setMax (palette.getMax () + d * x);
}

void glgrib_field::scalePaletteDown (float x)
{
  if (! palette.hasMin ()) 
    palette.setMin (getNormedMinValue ()); 
  if (! palette.hasMax ()) 
    palette.setMax (getNormedMaxValue ()); 
  float d = palette.getMax () - palette.getMin ();
  palette.setMin (palette.getMin () + d * x);
  palette.setMax (palette.getMax () - d * x);
}

const glgrib_palette & glgrib_field::getPalette () const
{
  return palette;
}

const glgrib_options_field & glgrib_field::getOptions () const 
{ 
  opts.palette = palette.getOptions ();
  return opts; 
}
