#include "glGribBorder.h"

void glGrib::Border::setup (const glGrib::OptionsBorder & o)
{
  if (! o.on)
    return;

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
  glGrib::Lines::setup (opts.lines, mask, code);
}
