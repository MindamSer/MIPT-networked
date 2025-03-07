#include <enet/enet.h>
#include <stdio.h>

#include "settings.h"

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
    
    if (!(serverHost = enet_host_create(&address, 32, 2, 0, 0)))
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
        printf("%x:%u - packet received\n", event.peer->address.host, event.peer->address.port);
        printf("Data:\n'%s'\n", event.packet->data);
        enet_packet_destroy(event.packet);
        break;

      default:
        break;
      };
    }
  }
}