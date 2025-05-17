//
// Created by rgnter on 17/12/2024.
//

#ifndef WORLDTRACKER_HPP
#define WORLDTRACKER_HPP

#include "libserver/data/DataDefinitions.hpp"

#include <unordered_map>

namespace alicia
{

//!
using EntityId = uint16_t;
//!
constexpr EntityId InvalidEntityId = 0;

//!
class WorldTracker
{
public:
  //! An entity map.
  using EntityMap = std::unordered_map<soa::data::Uid, uint16_t>;

  //!
  EntityId AddCharacter(soa::data::Uid character);
  //!
  [[nodiscard]] EntityId GetCharacterEntityId(soa::data::Uid character);
  //!
  EntityId AddMount(soa::data::Uid mount);
  //!
  [[nodiscard]] EntityId GetMountEntityId(soa::data::Uid mount);

  //!
  [[nodiscard]] const EntityMap& GetMountEntities();
  //!
  [[nodiscard]] const EntityMap& GetCharacterEntities();

private:
  //! The next entity ID.
  EntityId _nextEntityId = 1;

  //! Mount entities in the world.
  EntityMap _mounts;
  //! Character entities in the world.
  EntityMap _characters;
};

} // namespace alicia

#endif //WORLDTRACKER_HPP
