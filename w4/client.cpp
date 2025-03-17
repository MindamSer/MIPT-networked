#include "client.h"
#include "protocol.h"

#include <iostream>
#include <unistd.h>


int Client::run()
{
  initWindow();

  if(initENet() != 0)
  {
    std::cout << "Cannot init network" << std::endl;
    return 1;
  }

  while(!WindowShouldClose()) 
  {
    dt = GetFrameTime();

    processMessages();
    updateEntities();
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
        on_new_entity_packet();
        printf("new it\n");
        break;
      case E_SERVER_TO_CLIENT_SET_CONTROLLED_ENTITY:
        on_set_controlled_entity();
        printf("got it\n");
        break;
      case E_SERVER_TO_CLIENT_SNAPSHOT:
        on_snapshot();
        break;
      default:
        break;
      };
      break;
    default:
      break;
    };
  }
}

void Client::updateEntities()
{
  if (my_entity != invalid_entity)
  {
    bool left = IsKeyDown(KEY_LEFT);
    bool right = IsKeyDown(KEY_RIGHT);
    bool up = IsKeyDown(KEY_UP);
    bool down = IsKeyDown(KEY_DOWN);
    get_entity(my_entity, [&](Entity& e)
    {
      // Update
      e.x += ((left ? -dt : 0.f) + (right ? +dt : 0.f)) * 100.f;
      e.y += ((up ? -dt : 0.f) + (down ? +dt : 0.f)) * 100.f;
      // Send
      send_entity_state(serverPeer, my_entity, e.x, e.y);
      camera.target.x = e.x;
      camera.target.y = e.y;
    });
  }
}

void Client::drawFrame()
{
  BeginDrawing();
  {
    ClearBackground(Color{40, 40, 40, 255});
    BeginMode2D(camera);
    {
      for (const Entity &e : entities)
      {
        const Rectangle rect = {e.x, e.y, 10.f, 10.f};
        DrawRectangleRec(rect, GetColor(e.color));
      }
    }
    EndMode2D();
  }
  EndDrawing();
}

void Client::on_new_entity_packet()
{
  ENetPacket* &packet = event.packet;

  Entity newEntity;
  deserialize_new_entity(packet, newEntity);
  auto itf = indexMap.find(newEntity.eid);
  if (itf != indexMap.end())
    return; // don't need to do anything, we already have entity
  indexMap[newEntity.eid] = entities.size();
  entities.push_back(newEntity);
}

void Client::on_set_controlled_entity()
{
  ENetPacket* &packet = event.packet;

  deserialize_set_controlled_entity(packet, my_entity);
}

template<typename Callable>
void Client::get_entity(uint16_t eid, Callable c)
{
  auto itf = indexMap.find(eid);
  if (itf != indexMap.end())
    c(entities[itf->second]);
}

void Client::on_snapshot()
{
  ENetPacket* &packet = event.packet;

  uint16_t eid = invalid_entity;
  float x = 0.f; float y = 0.f;
  deserialize_snapshot(packet, eid, x, y);
  get_entity(eid, [&](Entity& e)
  {
    e.x = x;
    e.y = y;
  });
}
