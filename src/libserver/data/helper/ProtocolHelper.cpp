//
// Created by rgnter on 31/05/2025.
//

#include "libserver/data/helper/ProtocolHelper.hpp"

namespace alicia
{

namespace protocol
{

void BuildProtocolCharacter(Character& protocolCharacter, const soa::data::Character& character)
{
  // Set the character parts.
  // These serial ID's can be found in the `_ClientCharDefaultPartInfo` table.
  // Each character has specific part serial IDs for each part type.
  protocolCharacter.parts = {
    .charId = static_cast<uint8_t>(character.parts.modelId()),
    .mouthSerialId = static_cast<uint8_t>(character.parts.mouthId()),
    .faceSerialId = static_cast<uint8_t>(character.parts.faceId()),
  };

  // Set the character appearance.
  protocolCharacter.appearance = {
    .headSize = static_cast<uint8_t>(character.appearance.headSize()),
    .height = static_cast<uint8_t>(character.appearance.height()),
    .thighVolume = static_cast<uint8_t>(character.appearance.thighVolume()),
    .legVolume = static_cast<uint8_t>(character.appearance.legVolume()),
  };
}

void BuildProtocolHorse(Horse& protocolHorse, const soa::data::Horse& horse)
{
  protocolHorse.uid = horse.uid();
  protocolHorse.tid = horse.tid();
  protocolHorse.name = horse.name();

  protocolHorse.rating = horse.rating();
  protocolHorse.clazz = static_cast<uint8_t>(horse.clazz());
  protocolHorse.val0 = 1;
  protocolHorse.grade = static_cast<uint8_t>(horse.grade());
  protocolHorse.growthPoints = static_cast<uint16_t>(horse.growthPoints());

  protocolHorse.val16 = 0xb8a167e4,
  protocolHorse.val17 = 0;

  protocolHorse.vals0 = {
    .stamina = 0x7d0,
    .attractiveness = 0x3c,
    .hunger = 0x21c,
    .val0 = 0x00,
    .val1 = 0x03E8,
    .val2 = 0x00,
    .val3 = 0x00,
    .val4 = 0x00,
    .val5 = 0x03E8,
    .val6 = 0x1E,
    .val7 = 0x0A,
    .val8 = 0x0A,
    .val9 = 0x0A,
    .val10 = 0x00,
  };

  protocolHorse.vals1 = {
    .val0 = 0x00,
    .val1 = 0x00,
    .dateOfBirth = 0xb8a167e4,
    .val3 = 0x02,
    .val4 = 0x00,
    .classProgression = static_cast<uint32_t>(horse.clazzProgress()),
    .val5 = 0x00,
    .potentialLevel = static_cast<uint8_t>(horse.potentialLevel()),
    .hasPotential = static_cast<uint8_t>(horse.potentialType() != 0),
    .potentialValue = static_cast<uint8_t>(horse.potentialLevel()),
    .val9 = 0x00,
    .luck = static_cast<uint8_t>(horse.luckState()),
    .hasLuck = static_cast<uint8_t>(horse.luckState() != 0),
    .val12 = 0x00,
    .fatigue = 0x00,
    .val14 = 0x00,
    .emblem = static_cast<uint16_t>(horse.emblem())};

  protocolHorse.parts = {
    .skinId = static_cast<uint8_t>(horse.parts.skinId()),
    .maneId = static_cast<uint8_t>(horse.parts.maneId()),
    .tailId = static_cast<uint8_t>(horse.parts.tailId()),
    .faceId = static_cast<uint8_t>(horse.parts.faceId())};

  protocolHorse.appearance = {
    .scale = static_cast<uint8_t>(horse.appearance.scale()),
    .legLength = static_cast<uint8_t>(horse.appearance.legLength()),
    .legVolume = static_cast<uint8_t>(horse.appearance.legVolume()),
    .bodyLength = static_cast<uint8_t>(horse.appearance.bodyLength()),
    .bodyVolume = static_cast<uint8_t>(horse.appearance.bodyVolume())};

  protocolHorse.stats = {
    .agility = horse.stats.agility(),
    .control = horse.stats.control(),
    .speed = horse.stats.speed(),
    .strength = horse.stats.strength(),
    .spirit = horse.stats.spirit()};

  protocolHorse.mastery = {
    .spurMagicCount = horse.mastery.spurMagicCount(),
    .jumpCount = horse.mastery.jumpCount(),
    .slidingTime = horse.mastery.slidingTime(),
    .glidingDistance = horse.mastery.glidingDistance(),
  };
}

void BuildProtocolItems(std::vector<Item>& protocolItems, const std::vector<soa::Record<soa::data::Item>>& items)
{
  // The character equipment items.
  for (const soa::Record<soa::data::Item>& itemRecord : items)
  {
    auto& protocolItem = protocolItems.emplace_back();
    itemRecord.Immutable([&protocolItem](const soa::data::Item& item)
    {
      protocolItem.uid = item.uid();
      protocolItem.tid = item.tid();
      protocolItem.count = item.count();
    });
  }
}

} // namespace protocol

} // namespace alicia