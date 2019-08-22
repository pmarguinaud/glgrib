#include "glgrib_border.h"

void glgrib_border::init (const glgrib_options_border & o)
{
  opts = o;
  std::vector<unsigned int> mask, code;

  if (opts.countries.on)
    {
      mask.push_back (0x000000ff); code.push_back (0x00000001);
    }
  if (opts.regions.on)
    {
      mask.push_back (0x000000ff); code.push_back (0x00000002);
    }
  if (opts.sea.on)
    {
      mask.push_back (0x000000ff); code.push_back (0x00000003);
    }
  glgrib_lines::init (opts.lines, mask, code);
}
