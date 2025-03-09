#pragma once

#include <string>
#include <raylib.h>
#include <sys/types.h>

struct PlayerInfo
{
    std::string nickname;
    Vector2 pos;
    u_int8_t ping;
};