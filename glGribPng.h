#pragma once

#include <string>

namespace glGrib
{

void WritePng (const std::string &, int, int, unsigned char *);
void ReadPng (const std::string &, int *, int *, unsigned char **);


}
