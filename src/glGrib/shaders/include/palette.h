#include "palette/buffer_index.h"

uniform int rgba_size = 0;
uniform bool rgba_fixed = false;
layout (std430, binding=palette_buffer_idx) buffer rgba_buffer
{
  vec4 rgba_[];
};

