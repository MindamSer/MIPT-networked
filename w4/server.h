#pragma once

#include <cstdint>
#include <enet/enet.h>

#include <vector>
#include <map>

#include "entity.h"


class Server
{
public:
  int run();

  int initENet();
  void disconnectENet();
  
  void processMessages();
  void updateEntities(float dt);
  void sendSnapshots();

  uint16_t create_random_entity();
  void on_join(ENetPacket *packet, ENetPeer *peer, ENetHost *host);
  void on_state(ENetPacket *packet);

private:
  ENetHost *serverHost;
  ENetEvent event;

  std::vector<Entity> entities;
  std::map<uint16_t, ENetPeer*> controlledMap;  

  uint32_t lastTime = 0;
  int numAi = 10;
};