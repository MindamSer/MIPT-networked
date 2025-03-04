#include <cstdlib>
#include <enet/enet.h>
#include <iostream>

#include"settings.h"


int main(int argc, const char **argv)
{
  if (enet_initialize() != 0)
  {
    std::cout << "Cannot init ENet" << std::endl;
    return 1;
  }
  atexit(enet_deinitialize);
  std::cout << "ENet initialized" << std::endl;


  ENetAddress address;
  ENetHost *lobby;

  address.host = ENET_HOST_ANY;
  address.port = LOBBY_PORT;
  if (!(lobby = enet_host_create(&address, 32, 2, 0, 0)))
  {
    std::cout << "Cannot create lobby" << std::endl;
    return 1;
  }
  std::cout << "ENet host created" << std::endl;
  

  std::cout << "Lobby is active!" << std::endl;


  while (true)
  {
    ENetEvent event;
    while (enet_host_service(lobby, &event, 10) > 0)
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
        printf("Packet received:\n'%s'\n", event.packet->data);
        enet_packet_destroy(event.packet);
        break;

      default:
        break;
      };
    }
  }


  enet_host_destroy(lobby);

  return 0;
}

