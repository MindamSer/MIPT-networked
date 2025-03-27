#pragma once

#include <math.h>


inline float clamp(float in, float min, float max)
{
  return in < min ? min : in > max ? max : in;
}

inline float sign(float in)
{
  return in > 0.f ? 1.f : in < 0.f ? -1.f : 0.f;
}

inline float tile_val(float val, float border)
{
  if (val < -border)
    return val + 2.f * border;
  else if (val > border)
    return val - 2.f * border;
  return val;
}
