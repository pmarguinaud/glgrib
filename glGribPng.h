#pragma once

#include <string>

void glGribWritePng (const std::string &, int, int, unsigned char *);
void glGribReadPng (const std::string &, int *, int *, unsigned char **);

