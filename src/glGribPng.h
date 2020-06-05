#pragma once

#include <string>
#include "glGribBuffer.h"

namespace glGrib
{

void WritePng (const std::string &, int, int, 
               const glGrib::BufferPtr<unsigned char> &);
void ReadPng (const std::string &, int *, int *, 
              glGrib::BufferPtr<unsigned char> &);


}
