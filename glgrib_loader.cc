#include "glgrib_loader.h"
#include "glgrib_geometry.h"

#include <stdio.h>
#include <stdlib.h>
#include <eccodes.h>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <string>
#include <errno.h>
#include <time.h>

extern "C"
{
#include "lfi/lfi_grok.h"
#include "lfi/lfi_type.h"
#include "lfi/lfi_args.h"
#include "lfi/lfi_fort.h"
extern void lfiouv_mt64_ (LFIOUV_ARGS_DECL);
extern void lfinfo_mt64_ (LFINFO_ARGS_DECL);
extern void lfilec_mt64_ (LFILEC_ARGS_DECL);
extern void lfifer_mt64_ (LFIFER_ARGS_DECL);
}

codes_handle * glgrib_handle_from_file (const std::string & f)
{
  codes_handle * h = NULL;

  int k = f.find_last_of ('%');

  std::string file, ext;

  if (k >= 0)
    {
      ext = f.substr (k+1);
      file = f.substr (0, k);
    }
  else
    {
      file = f;
    }


  lfi_grok_t type = lfi_grok (file.c_str (), file.length ());

  switch (type)
    {
      case LFI_NONE:
      case LFI_UNKN:
        {
          int err = 0;
          FILE * in = fopen (file.c_str (), "r");
          if (in == NULL)
            throw std::runtime_error (std::string ("Could not open ") + 
                                      file + std::string (" for reading"));
          while ((h = codes_handle_new_from_file (0, in, PRODUCT_GRIB, &err)))
            {
	      std::string e = ext;
	      while (e.length ())
	        {
                  int p;
		  p = e.find_first_of (',');
		  std::string m;

		  if (p == std::string::npos)
                    {
                      m = e;
		      e = "";
		    }
		  else
		    {
		      m = e.substr (0, p);
		      e = e.substr (p+1);
		    }

                  p = m.find_first_of ('=');

                  if (p == std::string::npos)
                    throw std::runtime_error (std::string ("Malformed GRIB selector ") + ext);

		  std::string key = m.substr (0, p), val0 = m.substr (p+1);
                  
                  if (! codes_is_defined (h, key.c_str ()))
                    goto next;

		  bool string = (val0[0] == '"') && (val0[val0.length ()-1] == '"');

		  if (string)
                    val0 = val0.substr (1, val0.length () - 2);
      
		  if (string)
                    {
		      size_t len = 128;
                      char tmp[len];
		      grib_get_string (h, key.c_str (), tmp, &len);
		      std::string val1 = std::string (tmp);
		      if (val0 != val1)
                        goto next;
		    }
		  else
		    {
		      long int v0, v1;

		      try
                        {
		          v0 = std::stoi (val0);
		        }
		      catch (...)
		        {
                          throw std::runtime_error (std::string ("Malformed GRIB selector ") + ext);
		        }

                      codes_get_long (h, key.c_str (), &v1);
		      if (v0 != v1)
		        goto next;
		    }
	        }
	      break;
next:
              codes_handle_delete (h);
	      h = NULL;
	    }
          fclose (in);

          if (h == NULL)
            throw std::runtime_error (std::string ("No match for ") + ext + std::string (" in file ") + file);
         
	}
	break;
      case LFI_PURE:
      case LFI_ALTM:
        {
          lficom_t lficomm;
          void * LFI = &lficomm;
	  integer64 IREP, INUMER = 77, INIMES = 0, INBARP = 0, INBARI = 0,
		    ILONG = 0, IPOSEX = 0;
          logical LLNOMM = fort_TRUE, LLERFA = fort_TRUE, LLIMST = fort_FALSE;
	  character * CLNOMF = (character*)file.c_str (), * CLSTTO = (character*)"OLD", 
		    * CLSTTC = (character*)"KEEP", * CLNOMA = (character*)ext.c_str ();
	  character_len CLNOMF_len = file.length (), CLSTTO_len = 3, CLSTTC_len = 4, 
			CLNOMA_len = ext.length ();
	  integer64 * ITAB = NULL;
          

	  strncpy (lficomm.cmagic, "LFI_FORT", 8);
	  lficomm.lfihl = NULL;

          lfiouv_mt64_ (LFI, &IREP, &INUMER, &LLNOMM, CLNOMF, CLSTTO, &LLERFA, &LLIMST, 
			&INIMES, &INBARP, &INBARI, CLNOMF_len, CLSTTO_len);

          if (IREP != 0)
            throw std::runtime_error (std::string ("Error opening file ") + file);

	  lfinfo_mt64_ (LFI, &IREP, &INUMER, CLNOMA, &ILONG, &IPOSEX, CLNOMA_len);

          if (IREP != 0)
            throw std::runtime_error (std::string ("File ") + file + 
			    std::string (" does not contains ") + ext);

	  ITAB = (integer64 *)malloc (8 * ILONG);
	  lfilec_mt64_ (LFI, &IREP, &INUMER, CLNOMA, ITAB, &ILONG, CLNOMA_len);

          if (IREP != 0)
            throw std::runtime_error (std::string ("Reading ") + ext + 
			    std::string (" in ") + f + std::string (" failed"));

	  lfifer_mt64_ (LFI, &IREP, &INUMER, CLSTTC, CLSTTC_len);

          if (IREP != 0)
            throw std::runtime_error (std::string ("Closing ") + f + std::string (" failed"));

          h = codes_handle_new_from_message_copy (0, ITAB + 3, 8 * (ILONG - 3));
	  free (ITAB);

          if (h == NULL)
            throw std::runtime_error (std::string ("Article ") + ext + std::string (" of file ") + file +
                                      std::string (" does not contain a GRIB message"));
         
	}
	break;
    }

  return h;
}

glgrib_field_float_buffer_ptr glgrib_loader::load (const std::vector<std::string> & file, float fslot, 
                                                   glgrib_field_metadata * meta, int mult, int base)
{
  int islot = (int)fslot;

  if (fslot == (float)islot)
    return load (file[mult*islot+base], meta);

  float alpha = 1.0f  - (fslot - (float)islot);

  const std::string file1 = file[mult*(islot+0)+base];
  const std::string file2 = file[mult*(islot+1)+base];

  const_glgrib_geometry_ptr geom1 = glgrib_geometry_load (file1);
  const_glgrib_geometry_ptr geom2 = glgrib_geometry_load (file2);

  if (! geom1->isEqual (*geom2))
    {
      throw std::runtime_error (std::string ("Vector components have different geometries : ") 
                                + file1 + ", " + file2);
    }

  glgrib_field_metadata meta1, meta2;

  glgrib_field_float_buffer_ptr val1 = load (file1, &meta1);
  glgrib_field_float_buffer_ptr val2 = load (file2, &meta2);

  int size = geom1->size ();
  glgrib_field_float_buffer_ptr val = new_glgrib_field_float_buffer_ptr (size);

  float valmin = std::numeric_limits<float>::max (), 
        valmax = std::numeric_limits<float>::min (), 
        valmis;
  bool valmis_ok = true;
  for (int i = 0; i < size; i++)
    {
      float v1 = (*val1)[i], v2 = (*val2)[i];
      if ((v1 == meta1.valmis) || (v2 == meta2.valmis))
        {
          (*val)[i] = valmis;
        }
      else
        {
          float v = alpha * (*val1)[i] + (1.0 - alpha) * (*val2)[i];
          (*val)[i] = v;
          valmin = std::min (v, valmin);
          valmax = std::max (v, valmax);
          valmis_ok = valmis_ok && (v != valmis);
        }
    }
  if (! valmis_ok)
    {
      valmis = valmax * 1.1;
      for (int i = 0; i < size; i++)
        {
          float v1 = (*val1)[i], v2 = (*val2)[i];
          if ((v1 == meta1.valmis) || (v2 == meta2.valmis))
            (*val)[i] = valmis;
        }
    }

  *meta = meta1;

  meta->term = glgrib_option_date::interpolate (meta1.term, meta2.term, alpha);
  meta->valmin = valmin;
  meta->valmax = valmax;
  meta->valmis = valmis;

  return val;
}

glgrib_field_float_buffer_ptr glgrib_loader::load (const std::string & file, glgrib_field_metadata * meta)
{

  // Search cache
  for (std::list<entry>::iterator it = cache.begin (); it != cache.end (); it++)
    {
      if (it->file == file)
        {
	  entry e = *it;
	  cache.erase (it);
          glgrib_field_float_buffer_ptr data = e.data;
	  *meta = e.meta;
	  cache.push_front (e);
          return data;
	}
    }

  codes_handle * h = glgrib_handle_from_file (file);

  size_t v_len = 0;
  codes_get_size (h, "values", &v_len);

  double vmis, vmin, vmax;
  codes_get_double (h, "missingValue", &vmis);
  codes_get_double (h, "minimum",      &vmin);
  codes_get_double (h, "maximum",      &vmax);
  double * v = (double *)malloc (sizeof (double) * v_len);
  codes_get_double_array (h, "values", v, &v_len);

  glgrib_field_float_buffer_ptr val = new_glgrib_field_float_buffer_ptr (v_len);

  for (int i = 0; i < v_len; i++)
    (*val)[i] = v[i];

  free (v);

  meta->valmis = vmis;
  meta->valmin = vmin;
  meta->valmax = vmax;

  meta->CLNOMA = "";
  if (codes_is_defined (h, "CLNOMA"))
    {
      size_t len;
      codes_get_length (h, "CLNOMA", &len);
      char CLNOMA[len+1];
      codes_get_string (h, "CLNOMA", CLNOMA, &len);
      meta->CLNOMA = std::string (CLNOMA);
    }

#define CODES_GET(attr) \
  do {                                             \
    meta->attr = 255;                              \
    if (codes_is_defined (h, #attr))               \
      codes_get_long (h, #attr, &meta->attr);      \
  } while (0)

  CODES_GET (discipline);
  CODES_GET (parameterCategory);
  CODES_GET (parameterNumber);
  CODES_GET (indicatorOfUnitOfTimeRange);
  CODES_GET (forecastTime);
#undef CODES_GET

#define CODES_GET(attr) \
  do {                                             \
    meta->base.attr = 255;                         \
    if (codes_is_defined (h, #attr))               \
      codes_get_long (h, #attr, &meta->base.attr); \
  } while (0)

  CODES_GET (year);
  CODES_GET (month);
  CODES_GET (day);
  CODES_GET (hour);
  CODES_GET (minute);
  CODES_GET (second);

#undef CODES_GET

  codes_handle_delete (h);

  switch (meta->indicatorOfUnitOfTimeRange)
    {
      case  0:        // m Minute 
        meta->forecastTerm = meta->forecastTime * 60.0;
        break;
      case  1:        // h Hour 
        meta->forecastTerm = meta->forecastTime *        3600.0;
        break;
      case  2:        // D Day 
        meta->forecastTerm = meta->forecastTime * 24.0 * 3600.0;
        break;
      case 10:        // 3h 3 hours 
        meta->forecastTerm = meta->forecastTime *  3.0 * 3600.0;
        break;
      case 11:        // 6h 6 hours 
        meta->forecastTerm = meta->forecastTime *  6.0 * 3600.0;
        break;
      case 12:        // 12h 12 hours 
        meta->forecastTerm = meta->forecastTime * 12.0 * 3600.0;
        break;
      case 13:        // s Second 
        meta->forecastTerm = meta->forecastTime;
        break;
      case 255:
        meta->forecastTerm = 0;
        break;
      default:
        throw std::runtime_error (std::string ("Unexpected indicatorOfUnitOfTimeRange found in `") + file + std::string ("'"));
    }

  meta->term = glgrib_option_date::date_from_t (glgrib_option_date::t_from_date (meta->base) + meta->forecastTerm);

  if (false)
    {
      std::cout << meta->indicatorOfUnitOfTimeRange << " " << meta->forecastTime << " " << meta->forecastTerm << std::endl;
      std::cout 
       << " " << meta->base.year  
       << " " << meta->base.month 
       << " " << meta->base.day   
       << " " << meta->base.hour  
       << " " << meta->base.minute
       << " " << meta->base.second
       << " " << std::endl;
      std::cout 
       << " " << meta->term.year  
       << " " << meta->term.month 
       << " " << meta->term.day   
       << " " << meta->term.hour  
       << " " << meta->term.minute
       << " " << meta->term.second
       << " " << std::endl;
    }


  if (this->size > 0)
    {
      entry e;
      e.file = file;
      e.data = val;
      e.meta = *meta;
     
      cache.push_front (e);
     
      // Reduce cache size
      while (cache.size () > this->size)
        cache.pop_back ();
    }

  return val;
}

