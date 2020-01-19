uniform vec3 scale0 = vec3 (1.0, 1.0, 1.0);

vec3 scalePosition (vec3 pos, vec3 normedPos, vec3 scale0)
{
  if (proj_vs == XYZ)
    {
      pos.x = scale0.x * normedPos.x;
      pos.y = scale0.y * normedPos.y;
      pos.z = scale0.z * normedPos.z;
    }
  else if (proj_vs == POLAR_SOUTH)
    {
      pos.x = pos.x - (scale0.x - 1.0f);
    }
  else
    {
      pos.x = pos.x + (scale0.x - 1.0f);
    }

  return pos;
}

float scalingFactor (vec3 normedPos)
{
  if (proj_vs == POLAR_SOUTH)
    {
      return 1.0 / (1.0 - normedPos.z);
    }
  else if (proj_vs == POLAR_NORTH)
    {
      return 1.0 / (1.0 + normedPos.z);
    }
  else if (proj_vs == MERCATOR)
    {
      return 1.0 / sqrt (1 - normedPos.z * normedPos.z);
    }
  return 1.0;
}

