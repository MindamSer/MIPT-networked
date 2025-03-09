#include <enet/enet.h>
#include <stdio.h>
#include <cstring>

#include "settings.h"


void sendPlayerInfoTo(ENetAddress *address, ENetPeer *peer)
{
  char *msg;
  sprintf(msg, "%u %hu", address->host, address->port);
  ENetPacket *packet = enet_packet_create(msg, strlen(msg) + 1, ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(peer, 1, packet);

  printf("sent server address to %x:%u\n", peer->address.host, peer->address.port);
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

  ENetHost *serverHost;
  {
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = SERVER_PORT;
    
    if (!(serverHost = enet_host_create(&address, 32, 3, 0, 0)))
    {
      printf("Cannot create server host\n");
      return 1;
    }
    printf("ENet server host created\n");
  }
  
  printf("Server is active!\n");





  ENetEvent event;

  while (true)
  {
    while (enet_host_service(serverHost, &event, 10) > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        printf("%x:%u - connecion established\n", event.peer->address.host, event.peer->address.port);
        break;

      case ENET_EVENT_TYPE_DISCONNECT:
        printf("%x:%u - disconnected\n", event.peer->address.host, event.peer->address.port);
        break;

      case ENET_EVENT_TYPE_RECEIVE:
        printf("%x:%u - packet received: '%s'\n", event.peer->address.host, event.peer->address.port, event.packet->data);
        enet_packet_destroy(event.packet);
        break;

      default:
        break;
      };
    }
  }


  enet_host_destroy(serverHost);

  return 0;
}