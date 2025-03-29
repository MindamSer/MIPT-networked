#include "protocol.h"

#include "BitStream.h"


void send_join(ENetPeer *peer)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(MessageType), ENET_PACKET_FLAG_RELIABLE);
  *packet->data = E_CLIENT_TO_SERVER_JOIN;

  enet_peer_send(peer, 1, packet);
}

void send_entity_input(ENetPeer *peer, const uint16_t &eid, const float &thr, const float &steer)
{
  size_t packetSize = sizeof(MessageType) + sizeof(uint16_t) + 2 * sizeof(float);

  ENetPacket *packet = enet_packet_create(nullptr, packetSize, ENET_PACKET_FLAG_UNSEQUENCED);
  BitStream bs(packetSize);

  bs << E_CLIENT_TO_SERVER_INPUT << eid << thr << steer;
  bs.flush(packet->data);

  enet_peer_send(peer, 1, packet);
}


void send_new_entity(ENetPeer *peer, const Entity &ent)
{
  size_t packetSize = sizeof(MessageType) + sizeof(Entity);

  ENetPacket *packet = enet_packet_create(nullptr, packetSize, ENET_PACKET_FLAG_RELIABLE);
  BitStream bs(packetSize);

  bs << E_SERVER_TO_CLIENT_NEW_ENTITY << ent;
  bs.flush(packet->data);

  enet_peer_send(peer, 0, packet);
}

void send_set_controlled_entity(ENetPeer *peer, const uint16_t &eid)
{
  size_t packetSize = sizeof(MessageType) + sizeof(uint16_t);

  ENetPacket *packet = enet_packet_create(nullptr, packetSize, ENET_PACKET_FLAG_RELIABLE);
  BitStream bs(packetSize);

  bs << E_SERVER_TO_CLIENT_SET_CONTROLLED_ENTITY << eid;
  bs.flush(packet->data);

  enet_peer_send(peer, 0, packet);
}

void send_time_msec(ENetPeer *peer, const uint32_t &timeMsec)
{
  size_t packetSize = sizeof(MessageType) + sizeof(uint32_t);

  ENetPacket *packet = enet_packet_create(nullptr, packetSize, ENET_PACKET_FLAG_RELIABLE);
  BitStream bs(packetSize);

  bs << E_SERVER_TO_CLIENT_TIME_MSEC << timeMsec;
  bs.flush(packet->data);

  enet_peer_send(peer, 0, packet);
}

void send_snapshot(ENetPeer *peer, const uint16_t &eid, const Snapshot &snap)
{
  size_t packetSize = sizeof(MessageType) + sizeof(Snapshot);

  ENetPacket *packet = enet_packet_create(nullptr, packetSize, ENET_PACKET_FLAG_UNSEQUENCED);
  BitStream bs(packetSize);

  bs << E_SERVER_TO_CLIENT_SNAPSHOT << snap;
  bs.flush(packet->data);

  enet_peer_send(peer, 0, packet);
}

void send_control_snapshot(ENetPeer *peer, const uint16_t &eid, ControlSnapshot &contSnap)
{
  size_t packetSize = sizeof(MessageType) + sizeof(ControlSnapshot);

  ENetPacket *packet = enet_packet_create(nullptr, packetSize, ENET_PACKET_FLAG_UNSEQUENCED);
  BitStream bs(packetSize);

  bs << E_SERVER_TO_CLIENT_SNAPSHOT << contSnap;
  bs.flush(packet->data);

  enet_peer_send(peer, 0, packet);
}


MessageType get_packet_type(ENetPacket *packet)
{
  return (MessageType)*packet->data;
}


void deserialize_entity_input(ENetPacket *packet, uint16_t &eid, float &thr, float &steer)
{
  size_t packetSize = sizeof(MessageType) + sizeof(uint16_t) + 2 * sizeof(float);

  BitStream bs(packet->data, packetSize);
  bs.skip(sizeof(MessageType));

  bs >> eid >> thr >> steer;
}


void deserialize_new_entity(ENetPacket *packet, Entity &ent)
{
  size_t packetSize = sizeof(MessageType) + sizeof(Entity);

  BitStream bs(packet->data, packetSize);
  bs.skip(sizeof(MessageType));

  bs >> ent;
}

void deserialize_set_controlled_entity(ENetPacket *packet, uint16_t &eid)
{
  size_t packetSize = sizeof(MessageType) + sizeof(uint16_t);

  BitStream bs(packet->data, packetSize);
  bs.skip(sizeof(MessageType));

  bs >> eid;
}

void deserialize_time_msec(ENetPacket *packet, uint32_t &timeMsec)
{
  size_t packetSize = sizeof(MessageType) + sizeof(uint32_t);

  BitStream bs(packet->data, packetSize);
  bs.skip(sizeof(MessageType));

  bs >> timeMsec;
}

void deserialize_snapshot(ENetPacket *packet, uint16_t &eid, Snapshot &snap)
{
  size_t packetSize = sizeof(MessageType) + sizeof(uint16_t) + sizeof(Snapshot);

  BitStream bs(packet->data, packetSize);
  bs.skip(sizeof(MessageType));

  bs >> eid >> snap;
}

void deserialize_control_snapshot(ENetPacket *packet, uint16_t &eid, ControlSnapshot &contSnap)
{
  size_t packetSize = sizeof(MessageType) + sizeof(uint16_t) + sizeof(ControlSnapshot);

  BitStream bs(packet->data, packetSize);
  bs.skip(sizeof(MessageType));

  bs >> eid >> contSnap;
}
