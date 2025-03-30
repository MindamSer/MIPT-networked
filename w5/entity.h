#pragma once

#include <cstdint>


constexpr uint16_t invalid_entity = -1;

struct Entity
{
  // immutable state
  uint16_t eid = invalid_entity;
  uint32_t color = 0xffffffff;

  // mutable state
  float x = 0.f;
  float y = 0.f;
  float vx = 0.f;
  float vy = 0.f;
  float alpha = 0.f;
  float omega = 0.f;

  // user input
  float thr = 0.f;
  float steer = 0.f;

  void update(float dt);
};


struct Snapshot
{
  uint32_t timeStamp = 0;

  float x = 0.f;
  float y = 0.f;
  float alpha = 0.f;
};

inline Snapshot interpolate(const Snapshot &a, const Snapshot &b, const uint32_t targetTime)
{
  float t = (1.f * (targetTime - a.timeStamp)) / (1.f * (b.timeStamp - a.timeStamp));
  return {targetTime, a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.alpha + (b.alpha - a.alpha) * t };
}

struct ControlSnapshot : public Snapshot
{
  float vx = 0.f;
  float vy = 0.f;
  float omega = 0.f;
};
