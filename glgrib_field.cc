#include "glgrib_field.h"
#include "glgrib_program.h"
#include "glgrib_palette.h"
#include "glgrib_field_scalar.h"
#include "glgrib_field_vector.h"
#include "glgrib_field_contour.h"
#include "glgrib_field_stream.h"
#include "glgrib_resolve.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <sqlite3.h>

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
  
  char options[512];
  glgrib_field_metadata meta;
  ld->load (opts_sql.path, 0, &meta);
  
  sqlite3 * db = NULL;
  sqlite3_stmt * req = NULL;
  int rc;
#define TRY(expr) do { if ((rc = expr) != SQLITE_OK) goto end; } while (0)
      
  TRY (sqlite3_open (glgrib_resolve (std::string ("glgrib.db")).c_str (), &db));
  
  if (meta.CLNOMA != "")
    {
      TRY (sqlite3_prepare_v2 (db, "SELECT options FROM CLNOMA2OPTIONS WHERE CLNOMA = ?;", -1, &req, 0));
      TRY (sqlite3_bind_text (req, 1, meta.CLNOMA.c_str (), strlen (meta.CLNOMA.c_str ()), NULL));
      if ((rc = sqlite3_step (req)) == SQLITE_ROW)
        goto step;
    }
  if ((meta.discipline != 255) && (meta.parameterCategory != 255) && (meta.parameterNumber != 255))
    {
      TRY (sqlite3_prepare_v2 (db, "SELECT options FROM GRIB2OPTIONS WHERE discipline = ? "
                                   "AND parameterCategory = ? AND parameterNumber = ?;", -1, 
      		       &req, 0));
  
      TRY (sqlite3_bind_int (req, 1, meta.discipline));
      TRY (sqlite3_bind_int (req, 2, meta.parameterCategory));
      TRY (sqlite3_bind_int (req, 3, meta.parameterNumber));
     
      if ((rc = sqlite3_step (req)) == SQLITE_ROW)
        goto step;
    }
  
  if (rc == SQLITE_DONE)
    rc = SQLITE_OK;
  
  opts_sql = *opts;
  
  goto end;
  
  
step:
  
  rc = SQLITE_OK;
  
  strncpy (options, (const char *)sqlite3_column_text (req, 0), sizeof (options));
  
  std::cout << " options = " << options << std::endl;
  
  opts_sql.parse_unseen (options);
  opts_sql.path = opts->path;
  
  std::cout << " opts_sql = " << opts_sql.asOption (opts_ref) << std::endl;
  
end:
  
  if (rc != SQLITE_OK)
    throw std::runtime_error (std::string (sqlite3_errmsg (db)));
  
  if (req != NULL)
    sqlite3_finalize (req);
  if (db != NULL)
    sqlite3_close (db);
      
  
#undef TRY


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

  std::cout << " save = " << options << std::endl;

  
  sqlite3 * db = NULL;
  sqlite3_stmt * req = NULL;
  int rc;
#define TRY(expr) do { if ((rc = expr) != SQLITE_OK) goto end; } while (0)

  TRY (sqlite3_open (glgrib_resolve (std::string ("glgrib.db")).c_str (), &db));

  if (meta[0].CLNOMA != "")
    {
      TRY (sqlite3_prepare_v2 (db, "INSERT OR REPLACE INTO CLNOMA2OPTIONS (CLNOMA, options) VALUES (?, ?);", -1, &req, 0));
      TRY (sqlite3_bind_text (req, 1, meta[0].CLNOMA.c_str (), strlen (meta[0].CLNOMA.c_str ()), NULL));
      TRY (sqlite3_bind_text (req, 2, options.c_str (), strlen (options.c_str ()), NULL));
      if ((rc = sqlite3_step (req)) == SQLITE_DONE)
        goto done;
    }
  if ((meta[0].discipline != 255) && (meta[0].parameterCategory != 255) && (meta[0].parameterNumber != 255))
    {
      TRY (sqlite3_prepare_v2 (db, "INSERT OR REPLACE INTO GRIB2OPTIONS (discipline, "
                                   "parameterCategory, parameterNumber, options) VALUES (?, ?, ?, ?);", -1, 
			       &req, 0));

      TRY (sqlite3_bind_int (req, 1, meta[0].discipline));
      TRY (sqlite3_bind_int (req, 2, meta[0].parameterCategory));
      TRY (sqlite3_bind_int (req, 3, meta[0].parameterNumber));
      TRY (sqlite3_bind_text (req, 4, options.c_str (), strlen (options.c_str ()), NULL));
     
      if ((rc = sqlite3_step (req)) == SQLITE_DONE)
        goto done;
    }

  goto end;

done:

  rc = SQLITE_OK;

end:

  if (rc != SQLITE_OK)
    throw std::runtime_error (std::string (sqlite3_errmsg (db)));

  if (req != NULL)
    sqlite3_finalize (req);
  if (db != NULL)
    sqlite3_close (db);
    

#undef TRY


}

