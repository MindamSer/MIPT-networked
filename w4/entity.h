#pragma once
#include <cstdint>

enum class EntityId : uint16_t
{
  Invalid = static_cast<uint16_t>(-1)
};

struct Entity
{
  EntityId eid = EntityId::Invalid;

  uint32_t color = 0xffffffff;
  float x = 0.f;
  float y = 0.f;
  float size = 10.f;

  uint32_t score = 0;

  bool serverControlled = false;
  float targetX = 0.f;
  float targetY = 0.f;
};

