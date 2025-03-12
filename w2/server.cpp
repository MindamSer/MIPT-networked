#include <enet/enet.h>
#include <stdint.h>

#include <string>
#include <format>
#include <iostream>

#include <map>

#include "settings.h"
#include "playerInfo.h"


void sendIDTo(uint32_t id, ENetPeer *peer)
{
  std::string msg = std::format("{}", id);
  ENetPacket *packet = enet_packet_create(msg.c_str(), msg.size() + 1, ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(peer, 0, packet);

  printf("Assigned ID %u to %x:%u\n", 
    id, peer->address.host, peer->address.port);
}

void sendNewPlayerTo(uint32_t id, PlayerInfo *playerInfo, ENetPeer *peer)
{
  std::string msg = std::format("c {} {}", id, playerInfo->nickname);
  ENetPacket *packet = enet_packet_create(msg.c_str(), msg.size() + 1, ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(peer, 1, packet);

  printf("Sent add player to %x:%u\n", 
    peer->address.host, peer->address.port);
}

void sendDelPlayerTo(uint32_t id, ENetPeer *peer)
{
  std::string msg = std::format("d {}", id);
  ENetPacket *packet = enet_packet_create(msg.c_str(), msg.size() + 1, ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(peer, 1, packet);

  printf("Sent delete player to %x:%u\n", 
    peer->address.host, peer->address.port);
}

void sendPlayerInfoTo(uint32_t id, PlayerInfo *playerInfo, ENetPeer *peer)
{
  std::string msg = std::format("{} {} {} {}", id, playerInfo->pos.x, playerInfo->pos.y, playerInfo->ping);
  ENetPacket *packet = enet_packet_create(msg.c_str(), msg.size() + 1, ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
  enet_peer_send(peer, 2, packet);

  printf("Sent player info to %x:%u\n", 
    peer->address.host, peer->address.port);
}

uint32_t getPlayerId(uint32_t port)
{
  return port;
}

std::string getPlayerName(uint32_t id)
{
  return std::format("jab-jabych#{}", id);
}



int main(int argc, const char **argv)
{
  if (enet_initialize() != 0)
  {
    std::cout << "Cannot init ENet" << std::endl;
    return 1;
  }
  atexit(enet_deinitialize);
  std::cout << "ENet initialized" << std::endl;


  ENetHost *serverHost;
  {
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = SERVER_PORT;
    
    if (!(serverHost = enet_host_create(&address, 32, 3, 0, 0)))
    {
      std::cout << "Cannot create ENet server host" << std::endl;
      return 1;
    }
    std::cout << "ENet server host created" << std::endl;
  }
  

  std::cout << "Server is active!" << std::endl;



  std::map<uint32_t, PlayerInfo> playerList;


  
  ENetEvent event;
  while (true)
  {
    while (enet_host_service(serverHost, &event, 10) > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        {
          printf("%x:%u - connecion established\n", 
            event.peer->address.host, event.peer->address.port);
          
          uint32_t newID = getPlayerId(event.peer->address.port);
          PlayerInfo newPI = {event.peer, getPlayerName(newID).c_str(), {}, 0};
          sendIDTo(newID, event.peer);

          for(auto IDplayerInfo : playerList)
          {
            sendNewPlayerTo(newID, &newPI, IDplayerInfo.second.peer);
            sendNewPlayerTo(IDplayerInfo.first, &IDplayerInfo.second, newPI.peer);
          }
          playerList.insert_or_assign(newID, newPI);
        }
        break;

      case ENET_EVENT_TYPE_DISCONNECT:
        {
          printf("%x:%u - disconnected\n", 
            event.peer->address.host, event.peer->address.port);

          uint32_t delID = getPlayerId(event.peer->address.port);

          playerList.erase(delID);
          for(auto IDplayerInfo : playerList)
          {
            sendDelPlayerTo(delID, IDplayerInfo.second.peer);
          }

          event.peer->data = nullptr;
        }
        break;

      case ENET_EVENT_TYPE_RECEIVE:
        {
          printf("%x:%u - packet received: '%s'\n", 
            event.peer->address.host, event.peer->address.port, event.packet->data);

          char *msgData;
          sprintf(msgData, "%s", event.packet->data);

          switch(event.channelID)
          {
          case 0:
            break;
          case 1:
            break;
          case 2:
            {
              uint32_t playerID;
              float x, y;
              sscanf(msgData, "%u %f %f", &playerID, &x, &y);

              auto &playerToUpdate = playerList[playerID];
              playerToUpdate.pos.x = x;
              playerToUpdate.pos.y = y;
              for(auto IDplayerInfo : playerList)
              {
                sendPlayerInfoTo(playerID, &playerToUpdate, IDplayerInfo.second.peer);
              }
            }
            break;
          default:
            break;
          }

          enet_packet_destroy(event.packet);
        }
        break;

      default:
        break;
      };
    }
  }


  enet_host_destroy(serverHost);

  return 0;
}