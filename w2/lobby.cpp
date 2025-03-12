#include <enet/enet.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>

#include <unordered_set>

#include "settings.h"


void sendAdressTo(ENetAddress *address, ENetPeer *peer)
{
  char *msg;
  sprintf(msg, "%u %hu", address->host, address->port);
  ENetPacket *packet = enet_packet_create(msg, strlen(msg) + 1, ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(peer, 0, packet);

  printf("Sent server address to %x:%u\n", 
    peer->address.host, peer->address.port);
}



int main(int argc, const char **argv)
{
  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet\n");
    return 1;
  }
  atexit(enet_deinitialize);
  printf("ENet initialized\n");


  ENetHost *lobbyHost;
  {
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = LOBBY_PORT;
    
    if (!(lobbyHost = enet_host_create(&address, 32, 1, 0, 0)))
    {
      printf("Cannot create ENet lobby host\n");
      return 1;
    }
    printf("ENet lobby host created\n");
  }

  
  printf("Lobby is active!\n");



  ENetAddress serverAddress;
  enet_address_set_host(&serverAddress, SERVER_ADDRESS);
  serverAddress.port = SERVER_PORT;

  std::unordered_set<ENetPeer*> playerPool;
  bool gameStarted = false;



  ENetEvent event;
  while (true)
  {
    while (enet_host_service(lobbyHost, &event, 10) > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        {
          printf("%x:%u - connecion established\n", 
            event.peer->address.host, event.peer->address.port);
        
          playerPool.insert(event.peer);

          if(gameStarted)
          {
            sendAdressTo(&serverAddress, event.peer);
          }
        }
        break;

      case ENET_EVENT_TYPE_DISCONNECT:
        {
          printf("%x:%u - disconnected\n", 
            event.peer->address.host, event.peer->address.port);

          playerPool.erase(event.peer);

          event.peer->data = nullptr;
        }
        break;

      case ENET_EVENT_TYPE_RECEIVE:
        {
          printf("%x:%u - packet received: '%s'\n", 
            event.peer->address.host, event.peer->address.port, event.packet->data);

          char *msgData;
          sprintf(msgData, "%s", event.packet->data);

          if(!strcmp(msgData, COMMAND_START))
          {
            printf("Redirecting players...\n");
            for(ENetPeer *player : playerPool)
            {
              sendAdressTo(&serverAddress, player);
            }
            printf("Players redirected\n");
            gameStarted = true;
            printf("Game started\n");
          }
  
          enet_packet_destroy(event.packet);
        }
        break;

      default:
        break;
      };
    }
  }


  enet_host_destroy(lobbyHost);

  return 0;
}

