// initial skeleton is a clone from https://github.com/jpcy/bgfx-minimal-example
//
#include "raylib.h"
#include <enet/enet.h>
#include <stdio.h>
#include <math.h>
#include <unordered_map>

#include "entity.h"
#include "protocol.h"
#include "CyclicBuffer.h"


static std::unordered_map<uint16_t, Entity> entities;
static uint16_t my_entity = invalid_entity;

static constexpr const uint32_t INTERPOLATION_DELAY = 200;
static constexpr const size_t SNAPSHOT_HISTORY_MAX = 32;
static std::unordered_map<uint16_t, CyclycBuffer<Snapshot, SNAPSHOT_HISTORY_MAX>> snapshot_histories;

static constexpr const size_t STATES_HISTORY_MAX = 32;
static CyclycBuffer<ControlSnapshot, STATES_HISTORY_MAX> my_entity_states_history;

static uint32_t curTime = 0;
static uint32_t lastTime = 0;

void on_new_entity_packet(ENetPacket *packet)
{
  Entity newEntity;
  deserialize_new_entity(packet, newEntity);

  auto itf = entities.find(newEntity.eid);
  if (itf != entities.end())
    return;

  entities[newEntity.eid] = newEntity;
  snapshot_histories[newEntity.eid] = {{curTime, newEntity.x, newEntity.y, newEntity.alpha}};
}

void on_set_controlled_entity(ENetPacket *packet)
{
  deserialize_set_controlled_entity(packet, my_entity);

  Entity &ent = entities[my_entity];
  my_entity_states_history = {{curTime, ent.x, ent.y, ent.alpha, ent.vx, ent.vy, ent.omega}};
}

void on_snapshot(ENetPacket *packet)
{
  uint16_t eid = invalid_entity;
  Snapshot snap;
  deserialize_snapshot(packet, eid, snap);
  snapshot_histories[eid].push(snap);
}

void correct_local_states(const ControlSnapshot &contSnap)
{
  return;
}

void on_control_snapshot(ENetPacket *packet)
{
  uint16_t eid = invalid_entity;
  ControlSnapshot contSnap;
  deserialize_control_snapshot(packet, eid, contSnap);
  correct_local_states(contSnap);
}

static void on_time(ENetPacket *packet, ENetPeer* peer)
{
  uint32_t timeMsec;
  deserialize_time_msec(packet, timeMsec);
  enet_time_set(timeMsec + peer->lastRoundTripTime / 2);
}

static void draw_entity(const Entity& e)
{
  Snapshot drawing = {};
  uint32_t targetTime = curTime - INTERPOLATION_DELAY;

  if (e.eid == my_entity)
  {
    size_t stateHistorySize = my_entity_states_history.size();

    size_t lowerIdx = 0;
    for (lowerIdx = 0; lowerIdx < stateHistorySize; ++lowerIdx)
    {
      if (my_entity_states_history[lowerIdx].timeStamp < targetTime)
      {
        break;
      }
    }

    if (lowerIdx == 0)
    {
      drawing = my_entity_states_history[0];
    }
    else if (lowerIdx == stateHistorySize)
    {
      drawing = my_entity_states_history[stateHistorySize - 1];
    }
    else
    {
      drawing = interpolate(my_entity_states_history[lowerIdx], my_entity_states_history[lowerIdx - 1], targetTime);
    }
  }
  else
  {
    auto &curSnapshotHistory = snapshot_histories[e.eid];
    size_t curHistorySize = curSnapshotHistory.size();

    size_t lowerIdx = 0;
    for (lowerIdx = 0; lowerIdx < curHistorySize; ++lowerIdx)
    {
      if (curSnapshotHistory[lowerIdx].timeStamp < targetTime)
      {
        break;
      }
    }

    if (lowerIdx == 0)
    {
      drawing = curSnapshotHistory[0];
    }
    else if (lowerIdx == curHistorySize)
    {
      drawing = curSnapshotHistory[curHistorySize - 1];
    }
    else
    {
      drawing = interpolate(curSnapshotHistory[lowerIdx], curSnapshotHistory[lowerIdx - 1], targetTime);
    }
  }

  const float shipLen = 3.f;
  const float shipWidth = 2.f;
  const Vector2 fwd = Vector2{cosf(drawing.alpha), sinf(drawing.alpha)};
  const Vector2 left = Vector2{-fwd.y, fwd.x};
  DrawTriangle(Vector2{drawing.x + fwd.x * shipLen * 0.5f, drawing.y + fwd.y * shipLen * 0.5f},
               Vector2{drawing.x - fwd.x * shipLen * 0.5f - left.x * shipWidth * 0.5f, drawing.y - fwd.y * shipLen * 0.5f - left.y * shipWidth * 0.5f},
               Vector2{drawing.x - fwd.x * shipLen * 0.5f + left.x * shipWidth * 0.5f, drawing.y - fwd.y * shipLen * 0.5f + left.y * shipWidth * 0.5f},
               GetColor(e.color));
}

static void update_net(ENetHost* client, ENetPeer* serverPeer)
{
  size_t processedMsgs = 0;
  ENetEvent event;
  while (enet_host_service(client, &event, 10) > 0)
  {
    ++processedMsgs;

    switch (event.type)
    {
    case ENET_EVENT_TYPE_CONNECT:
      printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
      send_join(serverPeer);
      break;
    case ENET_EVENT_TYPE_RECEIVE:
      switch (get_packet_type(event.packet))
      {
      case E_SERVER_TO_CLIENT_NEW_ENTITY:
        on_new_entity_packet(event.packet);
        break;
      case E_SERVER_TO_CLIENT_SET_CONTROLLED_ENTITY:
        on_set_controlled_entity(event.packet);
        break;
      case E_SERVER_TO_CLIENT_SNAPSHOT:
        on_snapshot(event.packet);
        break;
      case E_SERVER_TO_CLIENT_CONTROL_SNAPSHOT:
        on_control_snapshot(event.packet);
        break;
      case E_SERVER_TO_CLIENT_TIME_MSEC:
        on_time(event.packet, event.peer);
        break;
      default:
        break;
      };
      enet_packet_destroy(event.packet);
      break;
    default:
      break;
    };

    if (processedMsgs >= 7) break;
  }
}

static void simulate_world(ENetPeer* serverPeer, float dt)
{
  if (my_entity != invalid_entity)
  {
    Entity &ent = entities[my_entity];

    bool left = IsKeyDown(KEY_LEFT);
    bool right = IsKeyDown(KEY_RIGHT);
    bool up = IsKeyDown(KEY_UP);
    bool down = IsKeyDown(KEY_DOWN);

    // Update
    float thr = (up ? 1.f : 0.f) + (down ? -1.f : 0.f);
    float steer = (left ? -1.f : 0.f) + (right ? 1.f : 0.f);

    ent.thr = thr;
    ent.steer = steer;
    ent.update(dt);

    my_entity_states_history.push({curTime, ent.x, ent.y, ent.alpha, ent.vx, ent.vy, ent.omega});

    // Send
    send_entity_input(serverPeer, my_entity, thr, steer);
  }
}

static void draw_world(const Camera2D& camera)
{
  BeginDrawing();
    ClearBackground(GRAY);
    BeginMode2D(camera);

      for (const auto &entEntry : entities)
      {
        draw_entity(entEntry.second);
      }

    EndMode2D();
  EndDrawing();
}

int main(int argc, const char **argv)
{
  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet");
    return 1;
  }

  ENetHost *clientHost = enet_host_create(nullptr, 1, 2, 0, 0);
  if (!clientHost)
  {
    printf("Cannot create ENet client\n");
    return 1;
  }

  ENetAddress address;
  enet_address_set_host(&address, "localhost");
  address.port = 10131;

  ENetPeer *serverPeer = enet_host_connect(clientHost, &address, 2, 0);
  if (!serverPeer)
  {
    printf("Cannot connect to server");
    return 1;
  }

  int width = 600;
  int height = 600;

  InitWindow(width, height, "w5 networked MIPT");

  const int scrWidth = GetMonitorWidth(0);
  const int scrHeight = GetMonitorHeight(0);
  if (scrWidth < width || scrHeight < height)
  {
    width = std::min(scrWidth, width);
    height = std::min(scrHeight - 150, height);
    SetWindowSize(width, height);
  }

  Camera2D camera = { {0, 0}, {0, 0}, 0.f, 1.f };
  camera.target = Vector2{ 0.f, 0.f };
  camera.offset = Vector2{ width * 0.5f, height * 0.5f };
  camera.rotation = 0.f;
  camera.zoom = 10.f;

  SetTargetFPS(60);               // Set our game to run at 60 frames-per-second


  float dt = 0.f;
  enet_time_set(0);

  while (!WindowShouldClose())
  {
    curTime = enet_time_get();
    if (curTime > lastTime)
    {
      dt = (curTime - lastTime) * 0.001f;
    }
    else
    {
      dt = 0.f;
    }
    lastTime = curTime;

    update_net(clientHost, serverPeer);
    simulate_world(serverPeer, dt);
    draw_world(camera);
  }

  CloseWindow();
  return 0;
}
