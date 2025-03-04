#include <raylib.h>
#include <enet/enet.h>
#include <cstring>
#include <iostream>

#include"settings.h"


void send_fragmented_packet(ENetPeer *peer)
{
  const char *baseMsg = "Stay awhile and listen. ";
  const size_t msgLen = strlen(baseMsg);

  const size_t sendSize = 2500;
  char *hugeMessage = new char[sendSize];
  for (size_t i = 0; i < sendSize; ++i)
    hugeMessage[i] = baseMsg[i % msgLen];
  hugeMessage[sendSize-1] = '\0';

  ENetPacket *packet = enet_packet_create(hugeMessage, sendSize, ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(peer, 0, packet);

  delete[] hugeMessage;
}

void send_micro_packet(ENetPeer *peer)
{
  const char *msg = "dv/dt";
  ENetPacket *packet = enet_packet_create(msg, strlen(msg) + 1, ENET_PACKET_FLAG_UNSEQUENCED);
  enet_peer_send(peer, 1, packet);
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

    SetTargetFPS(APP_FPS);
  }
  std::cout << "Window initialized" << std::endl;


  if (enet_initialize() != 0)
  {
    std::cout << "Cannot init ENet" << std::endl;
    return 1;
  }
  atexit(enet_deinitialize);
  std::cout << "ENet initialized" << std::endl;


  ENetHost *client = enet_host_create(nullptr, 1, 2, 0, 0);
  if (!client)
  {
    std::cout << "Cannot create ENet client" << std::endl;
    return 1;
  }
  std::cout << "ENet client created" << std::endl;


  ENetAddress address;
  ENetPeer *lobbyPeer;

  enet_address_set_host(&address, LOBBY_ADDRESS);
  address.port = LOBBY_PORT;
  if (!(lobbyPeer = enet_host_connect(client, &address, 2, 0)))
  {
    std::cout << "Cannot connect to lobby" << std::endl;
    return 1;
  }
  std::cout << "Connected to lobby" << std::endl;


  uint32_t timeStart = enet_time_get();
  uint32_t lastFragmentedSendTime = timeStart;
  uint32_t lastMicroSendTime = timeStart;
  bool connected = false;
  float posx = GetRandomValue(50, width - 50);
  float posy = GetRandomValue(50, height - 50);
  float velx = 0.f;
  float vely = 0.f;


  while (!WindowShouldClose())
  {
    const float dt = GetFrameTime();
    ENetEvent event;
    while (enet_host_service(client, &event, 10) > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
        connected = true;
        break;
      case ENET_EVENT_TYPE_RECEIVE:
        printf("Packet received '%s'\n", event.packet->data);
        enet_packet_destroy(event.packet);
        break;
      default:
        break;
      };
    }
    if (false)
    {
      uint32_t curTime = enet_time_get();
      if (curTime - lastFragmentedSendTime > 1000)
      {
        lastFragmentedSendTime = curTime;
        send_fragmented_packet(lobbyPeer);
      }
      if (curTime - lastMicroSendTime > 100)
      {
        lastMicroSendTime = curTime;
        send_micro_packet(lobbyPeer);
      }
    }
    bool left = IsKeyDown(KEY_LEFT);
    bool right = IsKeyDown(KEY_RIGHT);
    bool up = IsKeyDown(KEY_UP);
    bool down = IsKeyDown(KEY_DOWN);
    constexpr float accel = 150.f;
    velx += ((left ? -1.f : 0.f) + (right ? 1.f : 0.f)) * dt * accel;
    vely += ((up ? -1.f : 0.f) + (down ? 1.f : 0.f)) * dt * accel;
    posx += velx * dt;
    posy += vely * dt;
    velx *= 0.99f;
    vely *= 0.99f;

    BeginDrawing();
      ClearBackground(BLACK);
      DrawText(TextFormat("Current status: %s", "unknown"), 20, 20, 20, WHITE);
      DrawText(TextFormat("My position: (%d, %d)", (int)posx, (int)posy), 20, 40, 20, WHITE);
      DrawText("List of players:", 20, 60, 20, WHITE);
      DrawCircleV(Vector2{posx, posy}, 10.f, WHITE);
    EndDrawing();
  }


  return 0;
}
