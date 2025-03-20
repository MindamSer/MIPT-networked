#pragma once

#include <enet/enet.h>
#include "raylib.h"

#include <unordered_map>

#include "entity.h"


class Client
{
public:
  int run();
  
  void initWindow();
  int  initENet();
  void disconnectENet();
  
  void processMessages();
  void updateMyEntity(float dt);
  void drawFrame();

  void onSetControlledEntity();
  void onNewEntity();
  void onSnapshot();
  void onScore();

private:
  int windowWidth = 800;
  int windowHeight = 600;
  Camera2D camera;

  ENetHost *clientHost;
  ENetPeer *serverPeer;
  bool connected = false;
  ENetEvent event;

  std::unordered_map<EntityId, Entity> entities;
  EntityId my_entity = EntityId::Invalid;
};