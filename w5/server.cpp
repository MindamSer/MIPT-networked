#include <enet/enet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>

#include "entity.h"
#include "protocol.h"


static std::unordered_map<uint16_t, Entity> entities;
static std::unordered_map<uint16_t, ENetPeer*> playerPeers;

static void on_join(ENetPacket *packet, ENetPeer *peer, ENetHost *host)
{
  // find max eid
  uint16_t newEid = entities.empty() ? invalid_entity : entities.begin()->first;
  for (const auto &entEntry : entities)
  {
    newEid = std::max(newEid, entEntry.first);
  }
  ++newEid;

  // make new entity
  Entity ent = {};
  ent.eid = newEid;
  ent.color = 
  0xff000000 +
  0x00440000 * (rand() % 5) +
  0x00004400 * (rand() % 5) +
  0x00000044 * (rand() % 5);
  ent.x = (rand() % 4) * 5.f;
  ent.y = (rand() % 4) * 5.f;


  // send old entities to new player
  for (const auto &entEntry : entities)
  {
    send_new_entity(peer, entEntry.second);
  }

  entities[newEid] = ent;
  playerPeers[newEid] = peer;

  // send info about new entity to everyone
  for (const auto &peerEntry : playerPeers)
    send_new_entity(peerEntry.second, ent);

  // send info about controlled entity
  send_set_controlled_entity(peer, newEid);
}

static void on_input(ENetPacket *packet)
{
  uint16_t eid = invalid_entity;
  float thr = 0.f; float steer = 0.f;
  deserialize_entity_input(packet, eid, thr, steer);

  Entity &ent = entities[eid];
  ent.thr = thr;
  ent.steer = steer;
}

static void update_net(ENetHost* serverHost)
{
  ENetEvent event;
  while (enet_host_service(serverHost, &event, 10) > 0)
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
        case E_CLIENT_TO_SERVER_INPUT:
          on_input(event.packet);
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

static void update_world(ENetHost* server, float dt)
{
  for (auto &entEntry : entities)
  {
    Entity &ent = entEntry.second;

    // simulate
    ent.update(dt);

    // send
    for (const auto &peerEntry : playerPeers)
    {
      send_snapshot(peerEntry.second, ent.eid, ent.x, ent.y, ent.alpha);
    }
  }
}

static void update_time(ENetHost* server, uint32_t curTime)
{
  for (const auto peerEntry : playerPeers)
  {
    send_time_msec(peerEntry.second, curTime);
  }
}

int main(int argc, const char **argv)
{
  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet");
    return 1;
  }
  atexit(enet_deinitialize);

  ENetHost *serverHost;
  {
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = 10131;

    serverHost = enet_host_create(&address, 32, 2, 0, 0);
    if (!serverHost)
    {
      printf("Cannot create ENet server\n");
      return 1;
    }
  }

  uint32_t lastTime = enet_time_get();
  float dt = 0.f;
  while (true)
  {
    uint32_t curTime = enet_time_get();
    dt = (curTime - lastTime) * 0.001f;
    lastTime = curTime;

    update_net(serverHost);
    update_world(serverHost, dt);
    update_time(serverHost, curTime);
  }

  enet_host_destroy(serverHost);

  return 0;
}
