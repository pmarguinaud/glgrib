#pragma once

#include <string>
#include "glGrib/Buffer.h"

namespace glGrib
{

void WritePng (const std::string &, int, int, 
               const glGrib::BufferPtr<unsigned char> &);
void ReadPng (const std::string &, int *, int *, 
              glGrib::BufferPtr<unsigned char> &);


}
