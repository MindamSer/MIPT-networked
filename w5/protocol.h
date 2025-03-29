#pragma once

#include <enet/enet.h>
#include <cstdint>

#include "entity.h"

enum MessageType : uint8_t
{
  E_CLIENT_TO_SERVER_JOIN = 0,
  E_CLIENT_TO_SERVER_INPUT,
  E_SERVER_TO_CLIENT_NEW_ENTITY,
  E_SERVER_TO_CLIENT_SET_CONTROLLED_ENTITY,
  E_SERVER_TO_CLIENT_TIME_MSEC,
  E_SERVER_TO_CLIENT_SNAPSHOT,
  E_SERVER_TO_CLIENT_CONTROL_SNAPSHOT
};

void send_join(ENetPeer *peer);
void send_entity_input(ENetPeer *peer, const uint16_t &eid, const float &thr, const float &steer);

void send_new_entity(ENetPeer *peer, const Entity &ent);
void send_set_controlled_entity(ENetPeer *peer, const uint16_t &eid);
void send_time_msec(ENetPeer *peer, const uint32_t &timeMsec);
void send_snapshot(ENetPeer *peer, const uint16_t &eid, const Snapshot &snap);
void send_control_snapshot(ENetPeer *peer, const uint16_t &eid, const ControlSnapshot &contSnap);

MessageType get_packet_type(ENetPacket *packet);

void deserialize_entity_input(ENetPacket *packet, uint16_t &eid, float &thr, float &steer);

void deserialize_new_entity(ENetPacket *packet, Entity &ent);
void deserialize_set_controlled_entity(ENetPacket *packet, uint16_t &eid);
void deserialize_time_msec(ENetPacket *packet, uint32_t &timeMsec);
void deserialize_snapshot(ENetPacket *packet, uint16_t &eid, Snapshot &snap);
void deserialize_control_snapshot(ENetPacket *packet, uint16_t &eid, ControlSnapshot &contSnap);
