#include <cstdint>
#include <cstring>
#include <format>
#include <iostream>
#include <enet/enet.h>
#include <raylib.h>

#include <map>

#include "settings.h"
#include "playerInfo.h"


void sendStartCommandTo(ENetPeer *peer)
{
  std::string msg = COMMAND_START;
  ENetPacket *packet = enet_packet_create(msg.c_str(), msg.size() + 1, ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(peer, 0, packet);

  printf("Sent start command to %x:%u\n", 
    peer->address.host, peer->address.port);
}

void sendIdPositionTo(uint32_t id, Vector2 *pos, ENetPeer *peer)
{
  std::string msg = std::format("{} {} {}", id, pos->x, pos->y);
  ENetPacket *packet = enet_packet_create(msg.c_str(), msg.size() + 1, ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(peer, 2, packet);

  printf("Sent position to %x:%u\n", 
    peer->address.host, peer->address.port);
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
  uint32_t idOnServer = -1;

  playerList.insert_or_assign(idOnServer, PlayerInfo{nullptr, "me", {}, 0});



  ENetEvent event;
  while (!WindowShouldClose())
  {
    const float dt = GetFrameTime();
    
    while (enet_host_service(clientHost, &event, 10) > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        printf("%x:%u - connecion established\n", event.peer->address.host, event.peer->address.port);
        break;

      case ENET_EVENT_TYPE_DISCONNECT:
        printf("%x:%u - disconnected\n", event.peer->address.host, event.peer->address.port);
        event.peer->data = nullptr;
        break;

      case ENET_EVENT_TYPE_RECEIVE:
        {
          printf("%x:%u - packet received: '%s'\n", event.peer->address.host, event.peer->address.port, event.packet->data);

          char *msgData;
          sprintf(msgData, "%s", event.packet->data);

          switch(event.channelID)
          {
          case 0:
            {
              if (connected)
              {
                playerList.erase(idOnServer);
                sscanf(msgData, "%u", &idOnServer);
                playerList.insert_or_assign(idOnServer, PlayerInfo{nullptr, "me", {}, 0});
                printf("Got id from server: %u\n", idOnServer);
              }
              else
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
              }
            }
            break;
  
          case 1:
            {
              if (msgData[0] == 'c')
              {
                uint32_t playerID;
                char nickname[32];
                sscanf(msgData+2, "%u %s", &playerID, nickname);
  
                printf("Adding player \"%s\" (ID %u)\n", nickname, playerID);
                playerList.insert_or_assign(playerID, PlayerInfo{nullptr, nickname, {}, 0});
              }
              if (msgData[0] == 'd')
              {
                uint32_t playerID;
                sscanf(msgData+2, "%u", &playerID);
  
                printf("Deleting player \"%s\" (ID %u)\n", playerList[playerID].nickname.c_str(), playerID);
                playerList.erase(playerID);
              }
            }
            break;

          case 2:
            {
              uint32_t playerID, ping;
              float x, y;
              sscanf(msgData, "%u %f %f %u", &playerID, &x, &y, &ping);
  
              auto &playerInfo = playerList[playerID];
              playerInfo.pos.x = x;
              playerInfo.pos.y = y;
              playerInfo.ping = ping;
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

    if(connected && idOnServer != -1)
    {
      sendIdPositionTo(idOnServer, &position, serverPeer);
    }
    else
    {
      playerList[idOnServer].pos = position;
    }

    if (!connected && IsKeyPressed(KEY_ENTER))
    {
        sendStartCommandTo(lobbyPeer);
    }


    BeginDrawing();
    {
      ClearBackground(BLACK);
      DrawText(TextFormat("Current status: %s", idOnServer == -1 ? "lobby" : "server"), 20, 20, 20, WHITE);
      DrawText(TextFormat("My position: (%d, %d)", (int)position.x, (int)position.y), 20, 40, 20, WHITE);
      DrawText("List of players:", 20, 60, 20, WHITE);
      int i = 0;
      Color color;
      char* playerLine;
      for(auto IDPI : playerList)
      {
        sprintf(playerLine, "%s %hu", IDPI.second.nickname.c_str(), IDPI.second.ping);
        color = IDPI.first == idOnServer ? RED : WHITE;
        DrawText(playerLine, 40, 80 + 20 * i, 20, color);
        DrawCircleV(Vector2{IDPI.second.pos.x, IDPI.second.pos.y}, 10.f, color);
        ++i;
      }
    }
    EndDrawing();
  }


  enet_host_destroy(clientHost);

  return 0;
}
