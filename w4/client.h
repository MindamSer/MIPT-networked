#pragma once

#include <enet/enet.h>
#include "raylib.h"

#include <vector>
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
  void updateEntities();
  void drawFrame();

  void on_new_entity_packet();
  void on_set_controlled_entity();
  
  template<typename Callable>
  void get_entity(uint16_t eid, Callable c);
  void on_snapshot();

private:
  int windowWidth = 800;
  int windowHeight = 600;
  Camera2D camera;

  ENetHost *clientHost;
  ENetPeer *serverPeer;
  bool connected = false;
  ENetEvent event;

  std::vector<Entity> entities; 
  std::unordered_map<uint16_t, size_t> indexMap; 
  uint16_t my_entity = invalid_entity;

  float dt = 0.f;
};