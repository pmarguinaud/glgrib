#include "glGrib/Loader.h"
#include "glGrib/Trigonometry.h"
#include "glGrib/Geometry.h"
#include "glGrib/Palette.h"
#include "glGrib/Container.h"

#include <eccodes.h>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <string>
#include <errno.h>
#include <time.h>

namespace glGrib
{

HandlePtr Loader::handleFromFile (const std::string & f)
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


  Container * cont = Container::create (file);

  h = cont->getHandleByExt (ext);

  Container::remove (cont);

  HandlePtr ghp = std::make_shared<Handle>(h);

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

void Loader::load (BufferPtr<float> * ptr, const std::vector<OptionFieldRef> & file, const OptionsGeometry & opts_geom, 
		   float fslot, FieldMetadata * meta, int mult, int base, bool diff, const OptionsMissing & opts_missing)
{
  int islot = static_cast<int> (fslot);
  float a, b;

  if (diff)
    {
      a = +1.0f; 
      b = -1.0f;
    }
  else
    {
      b = fslot - static_cast<float> (islot);
      a = 1.0f  - b;
      if (fslot == static_cast<float> (islot))
        return load (ptr, file[mult*islot+base], opts_geom, meta, opts_missing);
    }


  const std::string file1 = file[mult*(islot+0)+base];
  const std::string file2 = file[mult*(islot+1)+base];

  const_GeometryPtr geom1 = Geometry::load (this, file1, opts_geom);
  const_GeometryPtr geom2 = Geometry::load (this, file2, opts_geom);

  if (! geom1->isEqual (*geom2))
    {
      throw std::runtime_error (std::string ("Vector components have different geometries : ") 
                                + file1 + ", " + file2);
    }

  FieldMetadata meta1, meta2;

  if (ptr == nullptr)
    {
      load (nullptr, file1, opts_geom, &meta1, opts_missing);
      *meta = meta1;
    }
  else
    {
      BufferPtr<float> val1, val2;

      load (&val1, file1, opts_geom, &meta1, opts_missing);
      load (&val2, file2, opts_geom, &meta2, opts_missing);
    
      int size = geom1->size ();
      BufferPtr<float> val = BufferPtr<float> (size);
    
      float valmin = +std::numeric_limits<float>::max (), 
            valmax = -std::numeric_limits<float>::max (), 
            valmis = 0.0;
      bool valmis_ok = true;
    
      for (int i = 0; i < size; i++)
        {
          float v1 = val1[i], v2 = val2[i];
          if ((v1 == meta1.valmis) || (v2 == meta2.valmis))
            {
              val[i] = valmis;
            }
          else
            {
              float v = a * val1[i] + b * val2[i];
              val[i] = v;
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
              float v1 = val1[i], v2 = val2[i];
              if ((v1 == meta1.valmis) || (v2 == meta2.valmis))
                val[i] = valmis;
            }
        }
    
      *meta = meta1;
    
      if (diff)
        {
          float valm = std::max (std::abs (valmin), std::abs (valmax));
          meta->valmin = - valm;
          meta->valmax = + valm;
          meta->valmis = valmis;
        }
      else
        {
          meta->term = OptionDate::interpolate (meta1.term, meta2.term, a);
          meta->valmin = valmin;
          meta->valmax = valmax;
          meta->valmis = valmis;
        }
    
      *ptr = val;
    }
}

void Loader::load (BufferPtr<float> * ptr, const OptionFieldRef & file, 
		   const OptionsGeometry & opts_geom,  
                   FieldMetadata * meta, const OptionsMissing & opts_missing)
{
  HandlePtr ghp = handleFromFile (file);
  codes_handle * h = ghp == nullptr ? nullptr : ghp->getCodesHandle ();

  size_t v_len = 0;
  codes_get_size (h, "values", &v_len);

  double vmis, vmin, vmax;
  codes_get_double (h, "missingValue", &vmis);
  codes_get_double (h, "minimum",      &vmin);
  codes_get_double (h, "maximum",      &vmax);

  if (ptr != nullptr)
    {
      Buffer<double> v (v_len);

      codes_get_double_array (h, "values", &v[0], &v_len);

      BufferPtr<float> val = BufferPtr<float> (v_len);

      float valmin = +std::numeric_limits<float>::max (), 
            valmax = -std::numeric_limits<float>::max ();

      for (size_t i = 0; i < v_len; i++)
        {
          val[i] = v[i];
	  if (opts_missing.on)
            {
              if (v[i] == vmis)
                val[i] = opts_missing.value;
	      else if (std::abs (val[i] - opts_missing.value) <= opts_missing.epsilon)
                val[i] = opts_missing.value;
	      else
                {
                  valmax = std::max (valmax, val[i]);
                  valmin = std::min (valmin, val[i]);
		}
	    }
	}
 
      *ptr = val;

      if (opts_missing.on)
        {
          vmax = valmax;
	  vmin = valmin;
	}
    }

  if (opts_missing.on)
    vmis = opts_missing.value;

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

  meta->term = OptionDate::date_from_t (OptionDate::tFromDate (meta->base) + meta->forecastTerm);

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
       << " " << OptionDate::tFromDate (meta->term)
       << " " << std::endl;
    }


}

void Loader::uv2nd 
  (const_GeometryPtr geometry,
   const BufferPtr<float> data_u, const BufferPtr<float> data_v,
   BufferPtr<float> & data_n,     BufferPtr<float> & data_d,
   const FieldMetadata & meta_u,  const FieldMetadata & meta_v,
   FieldMetadata & meta_n,        FieldMetadata & meta_d)
{
  data_n = BufferPtr<float> (geometry->getNumberOfPoints ());
  data_d = BufferPtr<float> (geometry->getNumberOfPoints ());

  meta_n = meta_u; // TODO : handle this differently
  meta_d = meta_u;
  
  meta_n.valmin = Palette::defaultMin ();
  meta_n.valmax = 0.0f;
  meta_d.valmin = -180.0f;
  meta_d.valmax = +180.0f;

#pragma omp parallel for
  for (int i = 0; i < geometry->getNumberOfPoints (); i++)
    if (data_u[i] == meta_u.valmis)
      {
        data_n[i] = meta_u.valmis;
        data_d[i] = meta_u.valmis;
      }
    else if (data_v[i] != meta_u.valmis)
      {
        data_n[i] = std::sqrt (data_u[i] * data_u[i] + data_v[i] * data_v[i]);
        data_d[i] = rad2deg * atan2 (data_v[i], data_u[i]);
      }
    else
      throw std::runtime_error ("Inconsistent domain definition for U/V");

  geometry->applyNormScale (data_n);

  for (int i = 0; i < geometry->getNumberOfPoints (); i++)
    if (data_u[i] != meta_u.valmis)
      {
        if (data_n[i] < meta_n.valmin)
          meta_n.valmin = data_n[i];
        if (data_n[i] > meta_n.valmax)
          meta_n.valmax = data_n[i];
      }
    else
      {
        data_n[i] = meta_u.valmis;
      }

  geometry->applyUVangle (data_d);

}

}

