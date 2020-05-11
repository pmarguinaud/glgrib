
uniform int rgba_size = 0;
uniform bool rgba_fixed = false;
layout (std430, binding=33) buffer rgba_buffer
{
  vec4 rgba_[];
};

