#include "glgrib_loader.h"
#include "glgrib_trigonometry.h"
#include "glgrib_geometry.h"
#include "glgrib_palette.h"

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

glgrib_handle_ptr glgrib_loader::handle_from_file (const std::string & f)
{
  // Search cache
  for (cache_t::iterator it = cache.begin (); it != cache.end (); it++)
    if (it->file == f)
      {
        entry_t e = *it;
        cache.erase (it);
	cache.push_front (e);
	return e.ghp;
      }

  codes_handle * h = nullptr;

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
          if (in == nullptr)
            throw std::runtime_error (std::string ("Could not open ") + 
                                      file + std::string (" for reading"));
          while ((h = codes_handle_new_from_file (0, in, PRODUCT_GRIB, &err)))
            {
	      std::string e = ext;

	      // Parse GRIB selector and see if current handle complies
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

		  // Key, value
		  std::string key = m.substr (0, p), val0 = m.substr (p+1);
                  
                  if (! codes_is_defined (h, key.c_str ()))
                    goto next;

		  // Handle string & integer values
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

	      // If we got here, then the GRIB message complies with the selector : keep GRIB handle and exit the loop
	      
	      break;
next:
	      // Free GRIB handle
              codes_handle_delete (h);
	      h = nullptr;
	    }
          fclose (in);

          if (h == nullptr)
            throw std::runtime_error (std::string ("No match for ") + ext + std::string (" in file ") + file);
         
	}
	break;
      case LFI_NETW:
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
	  integer64 * ITAB = nullptr;
          

	  strncpy (lficomm.cmagic, "LFI_FORT", 8);
	  lficomm.lfihl = nullptr;

          lfiouv_mt64_ (LFI, &IREP, &INUMER, &LLNOMM, CLNOMF, CLSTTO, &LLERFA, &LLIMST, 
			&INIMES, &INBARP, &INBARI, CLNOMF_len, CLSTTO_len);

          if (IREP != 0)
            throw std::runtime_error (std::string ("Error opening file ") + file);

	  lfinfo_mt64_ (LFI, &IREP, &INUMER, CLNOMA, &ILONG, &IPOSEX, CLNOMA_len);

          if (IREP != 0)
            throw std::runtime_error (std::string ("File ") + file + 
			    std::string (" does not contains ") + ext);

	  ITAB = new integer64[ILONG];
	  lfilec_mt64_ (LFI, &IREP, &INUMER, CLNOMA, ITAB, &ILONG, CLNOMA_len);

          if (IREP != 0)
            throw std::runtime_error (std::string ("Reading ") + ext + 
			    std::string (" in ") + f + std::string (" failed"));

	  lfifer_mt64_ (LFI, &IREP, &INUMER, CLSTTC, CLSTTC_len);

          if (IREP != 0)
            throw std::runtime_error (std::string ("Closing ") + f + std::string (" failed"));

          h = codes_handle_new_from_message_copy (0, ITAB + 3, 8 * (ILONG - 3));
	  delete [] ITAB;

          if (h == nullptr)
            throw std::runtime_error (std::string ("Article ") + ext + std::string (" of file ") + file +
                                      std::string (" does not contain a GRIB message"));
         
	}
	break;
    }

  glgrib_handle_ptr ghp = std::make_shared<glgrib_handle>(h);

  if (this->size > 0)
    {
      entry_t e;
      e.file = f;
      e.ghp = ghp;
      cache.push_front (e);
      // Reduce cache size
      while (cache.size () > this->size)
        cache.pop_back ();
    }

  return ghp;
}

void glgrib_loader::load (glgrib_field_float_buffer_ptr * ptr, const std::vector<std::string> & file, const glgrib_options_geometry & opts_geom, 
		          float fslot, glgrib_field_metadata * meta, int mult, int base, bool diff)
{
  int islot = (int)fslot;
  float a, b;

  if (diff)
    {
      a = +1.0f; 
      b = -1.0f;
    }
  else
    {
      b = fslot - (float)islot;
      a = 1.0f  - b;
      if (fslot == (float)islot)
        return load (ptr, file[mult*islot+base], opts_geom, meta);
    }


  const std::string file1 = file[mult*(islot+0)+base];
  const std::string file2 = file[mult*(islot+1)+base];

  const_glgrib_geometry_ptr geom1 = glgrib_geometry::load (this, file1, opts_geom);
  const_glgrib_geometry_ptr geom2 = glgrib_geometry::load (this, file2, opts_geom);

  if (! geom1->isEqual (*geom2))
    {
      throw std::runtime_error (std::string ("Vector components have different geometries : ") 
                                + file1 + ", " + file2);
    }

  glgrib_field_metadata meta1, meta2;

  if (ptr == nullptr)
    {
      load (nullptr, file1, opts_geom, &meta1);
      *meta = meta1;
    }
  else
    {
      glgrib_field_float_buffer_ptr val1, val2;

      load (&val1, file1, opts_geom, &meta1);
      load (&val2, file2, opts_geom, &meta2);
    
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
              float v = a * (*val1)[i] + b * (*val2)[i];
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
    
      if (diff)
        {
          float valm = std::max (fabs (valmin), fabs (valmax));
          meta->valmin = - valm;
          meta->valmax = + valm;
          meta->valmis = valmis;
          meta->CLNOMA            = "diff";
          meta->discipline        = 255;
          meta->parameterCategory = 255;
          meta->parameterNumber   = 255;
        }
      else
        {
          meta->term = glgrib_option_date::interpolate (meta1.term, meta2.term, a);
          meta->valmin = valmin;
          meta->valmax = valmax;
          meta->valmis = valmis;
        }
    
      *ptr = val;
    }
}

void glgrib_loader::load (glgrib_field_float_buffer_ptr * ptr, const std::string & file, 
		          const glgrib_options_geometry & opts_geom, glgrib_field_metadata * meta)
{
  glgrib_handle_ptr ghp = handle_from_file (file);
  codes_handle * h = ghp->getCodesHandle ();

  size_t v_len = 0;
  codes_get_size (h, "values", &v_len);

  double vmis, vmin, vmax;
  codes_get_double (h, "missingValue", &vmis);
  codes_get_double (h, "minimum",      &vmin);
  codes_get_double (h, "maximum",      &vmax);

  if (ptr != nullptr)
    {
      double * v = new double[v_len];
      codes_get_double_array (h, "values", v, &v_len);

      glgrib_field_float_buffer_ptr val = new_glgrib_field_float_buffer_ptr (v_len);

      for (int i = 0; i < v_len; i++)
        (*val)[i] = v[i];

      delete [] v;
      *ptr = val;
    }

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
       << " " << glgrib_option_date::t_from_date (meta->term)
       << " " << std::endl;
    }


}

void glgrib_loader::uv2nd (const_glgrib_geometry_ptr geometry,
                           const glgrib_field_float_buffer_ptr data_u, 
                           const glgrib_field_float_buffer_ptr data_v,
                           glgrib_field_float_buffer_ptr & data_n, 
                           glgrib_field_float_buffer_ptr & data_d,
                           const glgrib_field_metadata & meta_u, 
                           const glgrib_field_metadata & meta_v,
                           glgrib_field_metadata & meta_n, 
                           glgrib_field_metadata & meta_d)
{
  data_n = new_glgrib_field_float_buffer_ptr (geometry->getNumberOfPoints ());
  data_d = new_glgrib_field_float_buffer_ptr (geometry->getNumberOfPoints ());

  meta_n = meta_u; // TODO : handle this differently
  meta_d = meta_u;
  
  meta_n.valmin = glgrib_palette::defaultMin;
  meta_n.valmax = 0.0f;
  meta_d.valmin = -180.0f;
  meta_d.valmax = +180.0f;

#pragma omp parallel for
  for (int i = 0; i < geometry->getNumberOfPoints (); i++)
    if ((*data_u)[i] == meta_u.valmis)
      {
        (*data_n)[i] = meta_u.valmis;
        (*data_d)[i] = meta_u.valmis;
      }
    else if ((*data_v)[i] != meta_u.valmis)
      {
        (*data_n)[i] = sqrt ((*data_u)[i] * (*data_u)[i] + (*data_v)[i] * (*data_v)[i]);
        (*data_d)[i] = rad2deg * atan2 ((*data_v)[i], (*data_u)[i]);
      }
    else
      throw std::runtime_error ("Inconsistent domain definition for U/V");

  geometry->applyNormScale (data_n->data ());

  for (int i = 0; i < geometry->getNumberOfPoints (); i++)
    if ((*data_u)[i] != meta_u.valmis)
      {
        if ((*data_n)[i] < meta_n.valmin)
          meta_n.valmin = (*data_n)[i];
        if ((*data_n)[i] > meta_n.valmax)
          meta_n.valmax = (*data_n)[i];
      }
    else
      {
        (*data_n)[i] = meta_u.valmis;
      }

  geometry->applyUVangle (data_d->data ());

}



