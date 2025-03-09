#pragma once

#include <string>
#include <raylib.h>
#include <cstdint>

struct PlayerInfo
{
    std::string nickname;
    Vector2 pos;
    uint8_t ping;
};