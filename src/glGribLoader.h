#pragma once

#include <string>
#include <list>
#include <memory>

#include <eccodes.h>
#include "glGribFieldMetadata.h"
#include "glGribBuffer.h"
#include "glGribHandle.h"
#include "glGribGeometry.h"
#include "glGribOptions.h"

namespace glGrib
{

class Loader 
{
public:
  static void uv2nd (const_GeometryPtr,
                     const BufferPtr<float>, 
                     const BufferPtr<float>,
                     BufferPtr<float> &, 
                     BufferPtr<float> &,
                     const FieldMetadata &, 
                     const FieldMetadata &,
                     FieldMetadata &, 
                     FieldMetadata &);

  HandlePtr handleFromFile (const std::string &);
  void load (BufferPtr<float> *, const std::vector<OptionFieldRef> &, const OptionsGeometry &, 
            float, FieldMetadata *, int = 1, int = 0, bool diff = false);
  void load (BufferPtr<float> *, const OptionFieldRef &, const OptionsGeometry &, FieldMetadata *);
  void setSize (size_t _size) { size = _size; }
private:
  size_t size = 0;

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
