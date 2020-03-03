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

namespace glGrib
{

class Loader 
{
public:

  static void uv2nd (const_glGribGeometryPtr,
                     const FieldFloatBufferPtr, 
                     const FieldFloatBufferPtr,
                     FieldFloatBufferPtr &, 
                     FieldFloatBufferPtr &,
                     const FieldMetadata &, 
                     const FieldMetadata &,
                     FieldMetadata &, 
                     FieldMetadata &);

  HandlePtr handleFromFile (const std::string &);
  void load (FieldFloatBufferPtr *, const std::vector<std::string> &, const OptionsGeometry &, float, FieldMetadata *, int = 1, int = 0, bool diff = false);
  void load (FieldFloatBufferPtr *, const std::string &, const OptionsGeometry &, FieldMetadata *);
  void setSize (int _size) { size = _size; }
private:
  int size = 0;

  class entry_t
  {
  public:
    std::string file;
    HandlePtr ghp;
  };

  typedef std::list<entry_t> cache_t;
  cache_t cache;
};

}
