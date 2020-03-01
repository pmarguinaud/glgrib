#pragma once

#include <string>

void glgrib_write_png (const std::string &, int, int, unsigned char *);
void glgrib_read_png (const std::string &, int *, int *, unsigned char **);

