#include <cstdint>
#include <cstring>
#include <iostream>
#include <enet/enet.h>
#include <raylib.h>

#include <map>

#include "settings.h"
#include "playerInfo.h"


void sendStartCommandTo(ENetPeer *peer)
{
  const char *msg = COMMAND_START;
  ENetPacket *packet = enet_packet_create(msg, strlen(msg) + 1, ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(peer, 0, packet);

  printf("Sent start command to %x:%u\n", peer->address.host, peer->address.port);
}

void sendPositionTo(Vector2 *pos, ENetPeer *peer)
{
  char *msg;
  sprintf(msg, "%f %f", pos->x, pos->y);
  ENetPacket *packet = enet_packet_create(msg, strlen(msg) + 1, ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(peer, 2, packet);

  printf("Sent position to %x:%u\n", peer->address.host, peer->address.port);
}


int main(int argc, const char **argv)
{
  int width = WIN_W;
  int height = WIN_H;
  {
    InitWindow(width, height, WIN_TITLE);

    const int scrWidth = GetMonitorWidth(0);
    const int scrHeight = GetMonitorHeight(0);
    if (scrWidth < width || scrHeight < height)
    {
      width = std::min(scrWidth - WIN_BORD, width);
      height = std::min(scrHeight - WIN_BORD, height);
      SetWindowSize(width, height);
    }

    SetTargetFPS(WIN_FPS);
  }
  std::cout << "Window initialized" << std::endl;


  if (enet_initialize() != 0)
  {
    std::cout << "Cannot init ENet" << std::endl;
    return 1;
  }
  atexit(enet_deinitialize);
  std::cout << "ENet initialized" << std::endl;


  ENetHost *clientHost = enet_host_create(nullptr, 2, 3, 0, 0);
  if (!clientHost)
  {
    std::cout << "Cannot create client host" << std::endl;
    return 1;
  }
  std::cout << "ENet client host created" << std::endl;


  ENetPeer *lobbyPeer;
  ENetPeer *serverPeer;
  {
    ENetAddress address;
    enet_address_set_host(&address, LOBBY_ADDRESS);
    address.port = LOBBY_PORT;
    if (!(lobbyPeer = enet_host_connect(clientHost, &address, 1, 0)))
    {
      std::cout << "Cannot connect to lobby" << std::endl;
      return 1;
    }
    std::cout << "Connected to lobby" << std::endl;
  }

  printf("Client is active!\n");



  uint32_t timeStart = enet_time_get();
  Vector2 position;
  {
    float posx = GetRandomValue(50, width  - 50);
    float posy = GetRandomValue(50, height - 50);
    position = {posx, posy};
  }
  Vector2 velocity = {0.f, 0.f};
  bool connected = false;

  std::map<uint32_t, PlayerInfo> playerList;


  ENetEvent event;
  while (!WindowShouldClose())
  {
    const float dt = GetFrameTime();
    
    while (enet_host_service(clientHost, &event, 10) > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        {
          printf("%x:%u - connecion established\n", event.peer->address.host, event.peer->address.port);
          break;
        }

      case ENET_EVENT_TYPE_DISCONNECT:
        {
          printf("%x:%u - disconnected\n", event.peer->address.host, event.peer->address.port);
          break;
        }

      case ENET_EVENT_TYPE_RECEIVE:
        {
          printf("%x:%u - packet received: '%s'\n", event.peer->address.host, event.peer->address.port, event.packet->data);

          char *msgData;
          sprintf(msgData, "%s", event.packet->data);

          switch(event.channelID)
          {
          case 0:
            {
              std::cout << "Got server address, connecting..." << std::endl;
    
              ENetAddress address;
              sscanf(msgData, "%u %hu", &address.host, &address.port);
              if (!(serverPeer = enet_host_connect(clientHost, &address, 3, 0)))
              {
                std::cout << "Cannot connect to server" << std::endl;
                return 1;
              }
              connected = true;
  
              std::cout << "Connected to server" << std::endl;
              break;
            }
  
          case 1:
            {
              if (msgData[0] == 'c')
              {
                uint32_t playerID;
                char *nickname;
                sscanf(msgData+2, "%u %s", &playerID, nickname);
  
                printf("Addinf player \"%s\" (ID %u)\n", nickname, playerID);
                playerList.insert_or_assign(playerID, PlayerInfo{nickname, {}, 0});
              }
              if (msgData[0] == 'd')
              {
                uint32_t playerID;
                sscanf(msgData+2, "%u", &playerID);
  
                printf("Deleting player \"%s\" (ID %u)\n", playerList[playerID].nickname.c_str(), playerID);
                playerList.erase(playerID);
              }
              break;
            }

          case 2:
            {
              uint32_t playerID, ping;
              float x, y;
              sscanf(msgData, "%u %f %f %u", &playerID, &x, &y, &ping);
  
              auto &playerInfo = playerList[playerID];
              playerInfo.pos.x = x;
              playerInfo.pos.y = y;
              playerInfo.ping = ping;
              break;
            }

          default:
            break;
          }
  
          enet_packet_destroy(event.packet);
          break;
        }

      default:
        break;
      };
    }

    if(!connected && IsKeyPressed(KEY_ENTER))
    {
      sendStartCommandTo(lobbyPeer);
    }

    
    {
      bool left = IsKeyDown(KEY_LEFT);
      bool right = IsKeyDown(KEY_RIGHT);
      bool up = IsKeyDown(KEY_UP);
      bool down = IsKeyDown(KEY_DOWN);
      constexpr float accel = 150.f;
      velocity.x += ((left * -1.f) + (right * 1.f)) * dt * accel;
      velocity.y += ((up   * -1.f) + (down  * 1.f)) * dt * accel;
  
      position.x += velocity.x * dt;
      position.y += velocity.y * dt;
      velocity.x *= 0.99f;
      velocity.y *= 0.99f;
    }


    BeginDrawing();
    {
      ClearBackground(BLACK);
      DrawText(TextFormat("Current status: %s", !connected ? "lobby" : "server"), 20, 10, 10, WHITE);
      DrawText(TextFormat("My position: (%d, %d)", (int)position.x, (int)position.y), 20, 20, 10, WHITE);
      DrawText("List of players:", 20, 30, 10, WHITE);
      DrawCircleV(Vector2{position.x, position.y}, 10.f, WHITE);
    }
    EndDrawing();
  }


  return 0;
}
