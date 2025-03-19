#include "server.h"
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
    uint16_t eid = create_random_entity();
    entities[eid].serverControlled = true;
    controlledMap[eid] = nullptr;
  }

  lastTime = enet_time_get();

  float dt = 0.f;
  while(true)
  {
    uint32_t curTime = enet_time_get();
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
      printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
      break;
    case ENET_EVENT_TYPE_RECEIVE:
      switch (get_packet_type(event.packet))
      {
        case E_CLIENT_TO_SERVER_JOIN:
          on_join(event.packet, event.peer, serverHost);
          break;
        case E_CLIENT_TO_SERVER_STATE:
          on_state(event.packet);
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
  for (Entity &e : entities)
  {
    if (e.serverControlled)
    {
      const float diffX = e.targetX - e.x;
      const float diffY = e.targetY - e.y;
      const float dirX = diffX > 0.f ? 1.f : -1.f;
      const float dirY = diffY > 0.f ? 1.f : -1.f;
      constexpr float spd = 50.f;
      e.x += dirX * spd * dt;
      e.y += dirY * spd * dt;
      if (fabsf(diffX) < 10.f && fabsf(diffY) < 10.f)
      {
        e.targetX = (rand() % 40 - 20) * 15.f;
        e.targetY = (rand() % 40 - 20) * 15.f;
      }
    }
  }
}

void Server::sendSnapshots()
{
  for (const Entity &e : entities)
  {
    for (size_t i = 0; i < serverHost->peerCount; ++i)
    {
      ENetPeer *peer = &serverHost->peers[i];
      if (controlledMap[e.eid] != peer)
        send_snapshot(peer, e.eid, e.x, e.y);
    }
  }
}

uint16_t Server::create_random_entity()
{
  uint16_t newEid = entities.size();
  Entity ent(newEid);
  entities.push_back(ent);
  return newEid;
}

void Server::on_join(ENetPacket *packet, ENetPeer *peer, ENetHost *host)
{
    // send all entities
    for (const Entity &ent : entities)
    send_new_entity(peer, ent);

  // find max eid
  uint16_t newEid = create_random_entity();
  const Entity& ent = entities[newEid];

  controlledMap[newEid] = peer;


  // send info about new entity to everyone
  for (size_t i = 0; i < host->peerCount; ++i)
    send_new_entity(&host->peers[i], ent);
  // send info about controlled entity
  send_set_controlled_entity(peer, newEid);
}

void Server::on_state(ENetPacket *packet)
{
  uint16_t eid = invalid_entity;
  float x = 0.f; float y = 0.f;
  deserialize_entity_state(packet, eid, x, y);
  for (Entity &e : entities)
    if (e.eid == eid)
    {
      e.x = x;
      e.y = y;
    }
}
