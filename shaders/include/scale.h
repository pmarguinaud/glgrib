uniform float scale0 = 1.0f;

vec3 scalePosition (vec3 pos, vec3 normedPos, float scale0)
{
  if (proj == XYZ)
    {
      pos.x = scale0 * normedPos.x;
      pos.y = scale0 * normedPos.y;
      pos.z = scale0 * normedPos.z;
    }
  else if (proj == POLAR_SOUTH)
    {
      pos.x = pos.x - (scale0 - 1.0f);
    }
  else
    {
      pos.x = pos.x + (scale0 - 1.0f);
    }

  return pos;
}

float scalingFactor (vec3 normedPos)
{
  if (proj == POLAR_SOUTH)
    {
      return 1.0 / (1.0 - normedPos.z);
    }
  else if (proj == POLAR_NORTH)
    {
      return 1.0 / (1.0 + normedPos.z);
    }
  else if (proj == MERCATOR)
    {
      return 1.0 / sqrt (1 - normedPos.z * normedPos.z);
    }
  return 1.0;
}

