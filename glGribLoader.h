#pragma once

#include <string>
#include <list>
#include <memory>

#include <eccodes.h>
#include "glGribFieldMetadata.h"
#include "glGribFieldFloatBuffer.h"
#include "glGribHandle.h"
#include "glGribGeometry.h"
#include "glGribOptions.h"

class glGribLoader 
{
public:

  static void uv2nd (const_glGribGeometryPtr,
                     const glGribFieldFloatBufferPtr, 
                     const glGribFieldFloatBufferPtr,
                     glGribFieldFloatBufferPtr &, 
                     glGribFieldFloatBufferPtr &,
                     const glGribFieldMetadata &, 
                     const glGribFieldMetadata &,
                     glGribFieldMetadata &, 
                     glGribFieldMetadata &);

  glgrib_handle_ptr handleFromFile (const std::string &);
  void load (glGribFieldFloatBufferPtr *, const std::vector<std::string> &, const glGribOptionsGeometry &, float, glGribFieldMetadata *, int = 1, int = 0, bool diff = false);
  void load (glGribFieldFloatBufferPtr *, const std::string &, const glGribOptionsGeometry &, glGribFieldMetadata *);
  void setSize (int _size) { size = _size; }
private:
  int size = 0;

  class entry_t
  {
  public:
    std::string file;
    glgrib_handle_ptr ghp;
  };

  typedef std::list<entry_t> cache_t;
  cache_t cache;
};
