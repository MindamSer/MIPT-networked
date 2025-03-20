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
    checkCollisions();
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
  while (enet_host_service(serverHost, &event, 1) > 0)
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
    }
  }
}

void Server::checkCollisions()
{
  for (auto &entEntry1 : entities)
  {
    for (auto &entEntry2 : entities)
    {
      if(entEntry1.first != entEntry2.first)
      {
        Entity &ent1 = entEntry1.second;
        Entity &ent2 = entEntry2.second;

        float diffX = fabs((ent1.x + ent1.size / 2) - (ent2.x + ent2.size / 2)) - ent1.size / 2 - ent2.size / 2;
        float diffY = fabs((ent1.y + ent1.size / 2) - (ent2.y + ent2.size / 2)) - ent1.size / 2 - ent2.size / 2;

        if (diffX < 0.f && diffY < 0.f && ent1.size > ent2.size)
        {
          ent1.size += ent2.size / 2.f;
          ent1.score += ent2.size / 2;

          ent2.x = (rand() % 40 - 20) * 15.f;
          ent2.y = (rand() % 40 - 20) * 15.f;
          ent2.size = 5.f + rand() % 10 * 1.f;

          for (const auto &peerEntry : players)
          {
            send_entity_score(peerEntry.second, static_cast<uint16_t>(ent1.eid), ent1.size, ent1.score);
            send_entity_score(peerEntry.second, static_cast<uint16_t>(ent2.eid), ent2.size, ent2.score);
            send_snapshot(peerEntry.second, static_cast<uint16_t>(ent1.eid), ent1.x, ent1.y);
            send_snapshot(peerEntry.second, static_cast<uint16_t>(ent2.eid), ent2.x, ent2.y);
          }
        }
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

  ent.size = 5.f + rand() % 10 * 1.f;

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
