#include "glgrib_field.h"
#include "glgrib_program.h"
#include "glgrib_palette.h"
#include "glgrib_field_scalar.h"
#include "glgrib_field_vector.h"
#include "glgrib_field_contour.h"
#include "glgrib_field_stream.h"
#include "glgrib_resolve.h"
#include "glgrib_sqlite.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <algorithm>

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


void glgrib_field::getUserPref (glgrib_options_field * opts, glgrib_loader * ld)
{
  glgrib_options_field opts_sql = *opts;
  glgrib_options_field opts_ref;
  
  glgrib_field_metadata meta;
  ld->load (NULL, opts_sql.path, opts->geometry, 0, &meta);

  glgrib_sqlite db (glgrib_resolve ("glgrib.db"));
  
  std::string options;

  if (meta.CLNOMA != "")
    {
      glgrib_sqlite::stmt st = db.prepare ("SELECT options FROM CLNOMA2OPTIONS WHERE CLNOMA = ?;");
      st.bindall (&meta.CLNOMA);
      if (st.fetch_row (&options))
        goto found;
    }

  if ((meta.discipline != 255) && (meta.parameterCategory != 255) && (meta.parameterNumber != 255))
    {
      glgrib_sqlite::stmt st = db.prepare ("SELECT options FROM GRIB2OPTIONS WHERE discipline = ? "
		                           "AND parameterCategory = ? AND parameterNumber = ?;");
      st.bindall (&meta.discipline, &meta.parameterCategory, &meta.parameterNumber);
      if (st.fetch_row (&options))
        goto found;
    }
  
  return;

found:
  opts_sql.parse_unseen (options.c_str ());
  opts_sql.path = opts->path;
  *opts = opts_sql;
}

glgrib_field * glgrib_field::create (const glgrib_options_field & opts, float slot, glgrib_loader * ld)
{

  if (opts.path.size () == 0)
    return NULL;

  glgrib_options_field opts1 = opts;

  if (opts.user_pref.on)
    getUserPref (&opts1, ld);

  glgrib_field * fld = NULL;

  std::string type = opts1.type;

  std::transform (type.begin (), type.end (), type.begin (), ::toupper);

  if (type == "VECTOR")
    fld = new glgrib_field_vector ();
  else if (type == "STREAM")
    fld = new glgrib_field_stream ();
  else if (type == "CONTOUR")
    fld = new glgrib_field_contour ();
  else if (type == "SCALAR")
    fld = new glgrib_field_scalar ();
  else
    throw std::runtime_error (std::string ("Unknown field type : ") + type);

  fld->setup (ld, opts1, slot);


  return fld;
}

void glgrib_field::saveOptions () const
{
  glgrib_options_field opts1, opts2;

  opts1 = opts;
  opts1.path.clear ();
  std::string options = opts1.asOption (opts2);

  glgrib_sqlite db (glgrib_resolve ("glgrib.db"));

  if (meta[0].CLNOMA != "")
    {
      glgrib_sqlite::stmt st = db.prepare ("INSERT OR REPLACE INTO CLNOMA2OPTIONS (CLNOMA, options) VALUES (?, ?);");
      st.bindall (&meta[0].CLNOMA, &options);
      st.execute ();
    }
  if ((meta[0].discipline != 255) && (meta[0].parameterCategory != 255) && (meta[0].parameterNumber != 255))
    {
      glgrib_sqlite::stmt st = db.prepare ("INSERT OR REPLACE INTO GRIB2OPTIONS (discipline, "
                                           "parameterCategory, parameterNumber, options) VALUES (?, ?, ?, ?);");
      st.bindall (&meta[0].discipline, &meta[0].parameterCategory, &meta[0].parameterNumber, &options);
      st.execute ();
    }

}

