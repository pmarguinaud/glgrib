#include "glGribGrok.h"

extern "C"
{
#include "lfi_grok.h"
}

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>

glGrib::grok_t glGrib::Grok (const std::string & f)
{
  std::string file, ext;
  int k = f.find_last_of ('%');
  
  if (k >= 0)
    {   
      ext = f.substr (k+1);
      file = f.substr (0, k); 
    }   
  else
    {   
      file = f;
    }   

  switch (lfi_grok (file.c_str (), file.length ()))
    {
      case LFI_UNKN:
        break;
      default:
        return ext.length () > 0 ? grok_t::LFI_EXT : grok_t::LFI;
    }

  char head[4] = {0, 0, 0, 0};

  std::ifstream fh (file, std::ios::in | std::ios::binary);
  
  if (! fh.is_open ())
    goto unknown;

  if (! fh.read (&head[0], 4))
    goto unknown;

  if (strncmp (head, "GRIB", 4))
    goto unknown;

  return ext.length () > 0 ? grok_t::GRIB_EXT : grok_t::GRIB;
    
unknown:
  return grok_t::UNKNOWN;
}
