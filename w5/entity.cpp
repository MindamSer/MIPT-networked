#include "entity.h"
#include "mathUtils.h"


constexpr float worldSize = 30.f;

void Entity::update(float dt)
{
  float speed = thr * 3.f;
  float ang_spd = steer * 2.f;

  vx += cosf(alpha) * speed * dt;
  vy += sinf(alpha) * speed * dt;
  omega += ang_spd * dt;

  x += vx * dt;
  y += vy * dt;
  alpha += omega * dt;
  
  x = tile_val(x, worldSize);
  y = tile_val(y, worldSize);

  vx *= 0.999f;
  vy *= 0.999f;
  omega *= 0.99f;
}

