#include "protocol.h"
#include <cstring> // memcpy

#include "BitStream.h"


void send_join(ENetPeer *peer)
{
  uint64_t packetSize = sizeof(uint8_t);

  BitStream bs{packetSize};
  ENetPacket *packet = enet_packet_create(nullptr, packetSize, ENET_PACKET_FLAG_RELIABLE);

  bs << E_CLIENT_TO_SERVER_JOIN;
  bs.flush(packet->data);

  enet_peer_send(peer, 0, packet);
}

void send_new_entity(ENetPeer *peer, const Entity &ent)
{
  uint64_t packetSize = sizeof(uint8_t) + sizeof(Entity);

  BitStream bs{packetSize};
  ENetPacket *packet = enet_packet_create(nullptr, packetSize, ENET_PACKET_FLAG_RELIABLE);

  bs << E_SERVER_TO_CLIENT_NEW_ENTITY << ent;
  bs.flush(packet->data);

  enet_peer_send(peer, 0, packet);
}

void send_set_controlled_entity(ENetPeer *peer, uint16_t eid)
{
  uint64_t packetSize = sizeof(uint8_t) + sizeof(uint16_t);

  BitStream bs{packetSize};
  ENetPacket *packet = enet_packet_create(nullptr, packetSize, ENET_PACKET_FLAG_RELIABLE);
  
  bs << E_SERVER_TO_CLIENT_SET_CONTROLLED_ENTITY << eid;
  bs.flush(packet->data);

  enet_peer_send(peer, 0, packet);
}

void send_entity_state(ENetPeer *peer, uint16_t eid, float x, float y)
{
  uint64_t packetSize = sizeof(uint8_t) + sizeof(uint16_t) + 2 * sizeof(float);

  BitStream bs{packetSize};
  ENetPacket *packet = enet_packet_create(nullptr, packetSize, ENET_PACKET_FLAG_UNSEQUENCED);
  
  bs << E_CLIENT_TO_SERVER_STATE << eid << x << y;
  bs.flush(packet->data);

  enet_peer_send(peer, 1, packet);
}

void send_snapshot(ENetPeer *peer, uint16_t eid, float x, float y)
{
  uint64_t packetSize = sizeof(uint8_t) + sizeof(uint16_t) + 2 * sizeof(float);

  BitStream bs{packetSize};
  ENetPacket *packet = enet_packet_create(nullptr, packetSize, ENET_PACKET_FLAG_UNSEQUENCED);
  
  bs << E_SERVER_TO_CLIENT_SNAPSHOT << eid << x << y;
  bs.flush(packet->data);

  enet_peer_send(peer, 1, packet);
}

MessageType get_packet_type(ENetPacket *packet)
{
  return (MessageType)*packet->data;
}

void deserialize_new_entity(ENetPacket *packet, Entity &ent)
{
  uint64_t packetSize = sizeof(uint8_t) + sizeof(Entity);

  BitStream bs{packet->data, packetSize};

  bs.skip(sizeof(uint8_t));
  bs >> ent;
}

void deserialize_set_controlled_entity(ENetPacket *packet, uint16_t &eid)
{
  uint64_t packetSize = sizeof(uint8_t) + sizeof(uint16_t);

  BitStream bs{packet->data, packetSize};

  bs.skip(sizeof(uint8_t));
  bs >> eid;
}

void deserialize_entity_state(ENetPacket *packet, uint16_t &eid, float &x, float &y)
{
  uint64_t packetSize = sizeof(uint8_t) + sizeof(uint16_t) + 2 * sizeof(float);

  BitStream bs{packet->data, packetSize};

  bs.skip(sizeof(uint8_t));
  bs >> eid >> x >> y;
}

void deserialize_snapshot(ENetPacket *packet, uint16_t &eid, float &x, float &y)
{
  uint64_t packetSize = sizeof(uint8_t) + sizeof(uint16_t) + 2 * sizeof(float);

  BitStream bs{packet->data, packetSize};

  bs.skip(sizeof(uint8_t));
  bs >> eid >> x >> y;
}

