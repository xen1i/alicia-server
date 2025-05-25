//
// Created by rgnter on 25/05/2025.
//

#ifndef ROOMREGISTRY_HPP
#define ROOMREGISTRY_HPP

#include <cstdint>
#include <string>
#include <unordered_map>

namespace soa
{

struct Room
{
  uint32_t uid{};
  std::string name;
  std::string description;
  uint16_t missionId{};
  uint32_t otp{};

  uint8_t unk0;
  uint8_t unk1;
  uint8_t unk2;
  uint8_t unk3;
  uint16_t bitset;
  uint8_t unk4;
};

class RoomRegistry
{
public:
  Room& CreateRoom();
  Room& GetRoom(uint32_t uid);

  static RoomRegistry& Get()
  {
    static RoomRegistry singleton;
    return singleton;
  };

private:
  uint32_t _sequencedId = 0;
  std::unordered_map<uint32_t, Room> _rooms;
};

} // namespace soa

#endif //ROOMREGISTRY_HPP
