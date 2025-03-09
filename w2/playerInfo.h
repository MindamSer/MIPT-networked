#pragma once

#include "enet/enet.h"
#include <string>
#include <raylib.h>
#include <cstdint>

struct PlayerInfo
{
    ENetPeer *peer;
    std::string nickname;
    Vector2 pos;
    uint8_t ping;
};