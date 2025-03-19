#include "client.h"

#include <cstdint>
#include <iostream>
#include <unistd.h>

#include "entity.h"
#include "protocol.h"


int Client::run()
{
  initWindow();

  if(initENet() != 0)
  {
    std::cout << "Cannot init network" << std::endl;
    return 1;
  }

  float dt = 0.f;
  while(!WindowShouldClose()) 
  {
    dt = GetFrameTime();

    processMessages();
    updateMyEntity(dt);
    drawFrame();
  }
  CloseWindow();

  disconnectENet();

  return 0;
}


void Client::initWindow()
{
  InitWindow(windowWidth, windowHeight, "w4 networked MIPT");

  const int scrWidth = GetMonitorWidth(0);
  const int scrHeight = GetMonitorHeight(0);
  if (scrWidth < windowWidth || scrHeight < windowHeight)
  {
    windowWidth = std::min(scrWidth - 100, windowWidth);
    windowHeight = std::min(scrHeight - 100, windowHeight);
    SetWindowSize(windowWidth, windowHeight);
  }
  
  SetTargetFPS(60);

  camera = {};
  camera.offset = Vector2{ windowWidth * 0.5f, windowHeight * 0.5f };
  camera.target = Vector2{ 0.f, 0.f };
  camera.rotation = 0.f;
  camera.zoom = 1.f;
}

int Client::initENet()
{
  if (enet_initialize() != 0)
  {
    std::cout << "- Cannot init ENet" << std::endl;
    return 1;
  }
  atexit(enet_deinitialize);

  if (!(clientHost = enet_host_create(nullptr, 1, 2, 0, 0)))
  {
    std::cout << "- Cannot create ENet client host" << std::endl;
    return 1;
  }

  ENetAddress address;
  enet_address_set_host(&address, "localHost");
  address.port = 10000;

  if (!(serverPeer = enet_host_connect(clientHost, &address, 2, 0)))
  {
    std::cout << "- Cannot create ENet server peer" << std::endl;
    return 1;
  }

  return 0;
}

void Client::disconnectENet()
{
  enet_peer_disconnect(serverPeer, 0);
  enet_host_destroy(clientHost);
}


void Client::processMessages()
{
  while (enet_host_service(clientHost, &event, 0) > 0)
  {
    switch (event.type)
    {
    case ENET_EVENT_TYPE_CONNECT:
      printf("Connection with %x:%u established\n", 
        event.peer->address.host, event.peer->address.port);
      send_join(serverPeer);
      connected = true;
      break;
    
    case ENET_EVENT_TYPE_RECEIVE:
      switch (get_packet_type(event.packet))
      {
      case E_SERVER_TO_CLIENT_NEW_ENTITY:
        onNewEntity();
        std::cout << "New entity added" << std::endl;
        break;

      case E_SERVER_TO_CLIENT_SET_CONTROLLED_ENTITY:
        onSetControlledEntity();
        std::cout << "Controlled entity set" << std::endl;
        break;
        
      case E_SERVER_TO_CLIENT_SNAPSHOT:
        onSnapshot();
        break;

      default:
        break;
      };
      enet_packet_destroy(event.packet);
      break;

    default:
      break;
    };
  }
}

void Client::updateMyEntity(float dt)
{
  if (my_entity != EntityId::Invalid)
  {
    Entity &ent = entities[my_entity];

    bool left = IsKeyDown(KEY_LEFT);
    bool right = IsKeyDown(KEY_RIGHT);
    bool up = IsKeyDown(KEY_UP);
    bool down = IsKeyDown(KEY_DOWN);

    ent.x += ((left ? -dt : 0.f) + (right ? +dt : 0.f)) * 100.f;
    ent.y += ((up ? -dt : 0.f) + (down ? +dt : 0.f)) * 100.f;

    camera.target.x = ent.x;
    camera.target.y = ent.y;
    
    send_entity_state(serverPeer, static_cast<uint16_t>(my_entity), ent.x, ent.y);
  }
}

void Client::drawFrame()
{
  BeginDrawing();
  {
    ClearBackground(Color{40, 40, 40, 255});
    BeginMode2D(camera);
    {
      for (const auto &p : entities)
      {
        const Entity &ent = p.second;
        const Rectangle rect = {ent.x, ent.y, 10.f, 10.f};
        DrawRectangleRec(rect, GetColor(ent.color));
      }
    }
    EndMode2D();
  }
  EndDrawing();
}


void Client::onSetControlledEntity()
{
  ENetPacket* &packet = event.packet;

  uint16_t eid;
  deserialize_set_controlled_entity(packet, eid);

  my_entity = EntityId(eid);
}

void Client::onNewEntity()
{
  ENetPacket* &packet = event.packet;

  Entity newEntity;
  deserialize_new_entity(packet, newEntity);

  auto itf = entities.find(newEntity.eid);
  if (itf == entities.end())
  {
    entities.emplace(newEntity.eid, newEntity);
  }
}

void Client::onSnapshot()
{
  ENetPacket* &packet = event.packet;

  uint16_t eid;
  float x, y;
  deserialize_snapshot(packet, eid, x, y);
  
  Entity &ent = entities[EntityId(eid)];
  ent.x = x;
  ent.y = y;
}
