#pragma once

#include <cstdint>
#include <enet/enet.h>

#include <unordered_map>

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

  Entity createRandomEntity();

  void onJoin();
  void onState();

private:
  ENetHost *serverHost;
  ENetEvent event;

  std::unordered_map<EntityId, Entity> entities;
  std::unordered_map<EntityId, ENetPeer*> players;

  uint32_t lastTime = 0;
  int numAi = 10;
};