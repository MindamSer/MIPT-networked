#include "server.h"
#include "entity.h"
#include "protocol.h"

#include <iostream>
#include <cmath>

int Server::run()
{
  if(initENet() != 0)
  {
    std::cout << "Cannot init network" << std::endl;
    return 1;
  }

  for (int i = 0; i < numAi; ++i)
  {
    Entity ent = createRandomEntity();
    ent.serverControlled = true;

    entities[ent.eid] = ent;
  }

  lastTime = enet_time_get();

  uint32_t curTime = 0;
  float dt = 0.f;
  while(true)
  {
    curTime = enet_time_get();
    dt = (curTime - lastTime) * 0.001f;
    lastTime = curTime;

    processMessages();
    updateEntities(dt);
    sendSnapshots();
  }

  disconnectENet();

  return 0;
}

int Server::initENet()
{
  if (enet_initialize() != 0)
  {
    std::cout << "- Cannot init ENet" << std::endl;
    return 1;
  }
  atexit(enet_deinitialize);

  ENetAddress address;
  address.host = ENET_HOST_ANY;
  address.port = 10000;

  if (!(serverHost = enet_host_create(&address, 32, 2, 0, 0)))
  {
    std::cout << "- Cannot create ENet server host" << std::endl;
    return 1;
  }

  return 0;
}

void Server::disconnectENet()
{
  enet_host_destroy(serverHost);
}

void Server::processMessages()
{
  while (enet_host_service(serverHost, &event, 0) > 0)
  {
    switch (event.type)
    {
    case ENET_EVENT_TYPE_CONNECT:
      printf("Connection with %x:%u established\n",
        event.peer->address.host, event.peer->address.port);
      break;

    case ENET_EVENT_TYPE_RECEIVE:
      switch (get_packet_type(event.packet))
      {
        case E_CLIENT_TO_SERVER_JOIN:
          std::cout << "New player added" << std::endl;
          onJoin();
          break;

        case E_CLIENT_TO_SERVER_STATE:
          onState();
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

void Server::updateEntities(float dt)
{
  for (auto &entEntry : entities)
  {
    if (entEntry.second.serverControlled)
    {
      Entity &ent = entEntry.second;

      float diffX = ent.targetX - ent.x;
      float diffY = ent.targetY - ent.y;
      if (fabsf(diffX) < 10.f && fabsf(diffY) < 10.f)
      {
        ent.targetX = (rand() % 40 - 20) * 15.f;
        ent.targetY = (rand() % 40 - 20) * 15.f;

        diffX = ent.targetX - ent.x;
        diffY = ent.targetY - ent.y;
      }

      float dirX = diffX > 0.f ? 1.f : -1.f;
      float dirY = diffY > 0.f ? 1.f : -1.f;
      
      constexpr float spd = 50.f;
      ent.x += dirX * spd * dt;
      ent.y += dirY * spd * dt;
      if (fabsf(diffX) < 10.f && fabsf(diffY) < 10.f)
      {
        ent.targetX = (rand() % 40 - 20) * 15.f;
        ent.targetY = (rand() % 40 - 20) * 15.f;
      }
    }
  }
}

void Server::sendSnapshots()
{
  for (const auto &entEntry : entities)
  {
    for (const auto &peerEntry : players)
    {
      if (entEntry.first != peerEntry.first)
      {
        const Entity & ent = entEntry.second;
        send_snapshot(peerEntry.second, static_cast<uint16_t>(ent.eid), ent.x, ent.y);
      }
    }
  }
}

Entity Server::createRandomEntity()
{
  Entity ent;
  ent.eid = EntityId(entities.size());

  ent.color = 
  0x11000000 * (rand() % 12 + 1) +
  0x00110000 * (rand() % 12 + 1) +
  0x00001100 * (rand() % 12 + 1) +
  0x333333ff;
  ent.x = (rand() % 40 - 20) * 15.f;
  ent.y = (rand() % 40 - 20) * 15.f;

  ent.targetX = ent.x;
  ent.targetY = ent.y;

  return ent;
}

void Server::onJoin()
{
  for (const auto &entEntry : entities)
  {
    send_new_entity(event.peer, entEntry.second);
  }

  Entity ent = createRandomEntity();
  entities[ent.eid] = ent;
  players[ent.eid] = event.peer;

  for (const auto &peerEntry : players)
  {
    send_new_entity(peerEntry.second, ent);
  }
  send_set_controlled_entity(event.peer, static_cast<uint16_t>(ent.eid));
}

void Server::onState()
{
  ENetPacket* &packet = event.packet;

  uint16_t eid;
  float x, y;
  deserialize_entity_state(packet, eid, x, y);

  for (auto &entEntry : entities)
  {
    if(entEntry.first == EntityId(eid))
    {
      entEntry.second.x = x;
      entEntry.second.y = y;
    }
  }
}
