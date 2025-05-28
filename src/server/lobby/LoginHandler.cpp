/**
 * Alicia Server - dedicated server software
 * Copyright (C) 2024 Story Of Alicia
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 **/

#include "server/lobby/LoginHandler.hpp"

#include "server/lobby/LobbyDirector.hpp"
#include "spdlog/spdlog.h"

namespace alicia
{

LoginHandler::LoginHandler(
  LobbyDirector& lobbyDirector,
  CommandServer& server)
  : _server(server)
  , _lobbyDirector(lobbyDirector)
  , _dataDirector(_lobbyDirector.GetDataDirector())
{
}

void LoginHandler::Tick()
{
  // Process the login queue.
  while (not _clientLoginRequestQueue.empty())
  {
    const ClientId clientId = _clientLoginRequestQueue.front();
    const auto& loginContext = _clientLogins[clientId];

    // Load the user.
    auto user = _dataDirector.GetUsers().Get(loginContext.userName);
    if (not user)
      continue;

    _clientLoginRequestQueue.pop();

    bool isAuthenticated = false;
    bool hasCharacter = false;
    user->Immutable([&isAuthenticated, &hasCharacter, &loginContext](auto& user){
      isAuthenticated = user.token() == loginContext.userToken;
      hasCharacter == user.characterUid() != soa::data::InvalidUid;
    });

    // If the user succeeds in authentication queue user for further processing.
    if (isAuthenticated)
    {
      if (not hasCharacter)
      {
        QueueUserCreateNickname(clientId, loginContext.userName);
        break;
      }

      // Queue the processing of the response.
      _clientLoginResponseQueue.emplace(clientId);
    }
    else
    {
      QueueUserLoginRejected(clientId);
    }

    // Only one user login per tick.
    break;
  }

  while (not _clientLoginResponseQueue.empty())
  {
    const ClientId clientId = _clientLoginResponseQueue.front();
    const auto& loginContext = _clientLogins[clientId];

    {
      // Load the character.
      auto userRecord = _dataDirector.GetUsers().Get(loginContext.userName);
      assert(userRecord.has_value());

      auto user = *_dataDirector.GetUsers().Get(loginContext.userName);

      if (user().characterUid() != soa::data::InvalidUid)
      {
        auto character = _dataDirector.GetCharacters().Get(
          user().characterUid());
        if (not character)
        {
          continue;
        }

        // Load the character's equipment, horses and ranch.

        const auto characterEquipment = _dataDirector.GetItems().Get(
          (*character)().characterEquipment());
        const auto horseEquipment = _dataDirector.GetItems().Get(
          (*character)().horseEquipment());

        const auto horses = _dataDirector.GetHorses().Get((*character)().horseUids());
        const auto ranch = _dataDirector.GetRanches().Get((*character)().ranchUid());

        if (not characterEquipment
          || not horseEquipment
          || not horses
          || not ranch)
        {
          continue;
        }
      }
    }

    _clientLoginResponseQueue.pop();

    QueueUserLoginAccepted(clientId, loginContext.userName);
  }
}

void LoginHandler::HandleUserLogin(
  const ClientId clientId,
  const LobbyCommandLogin& login)
{
  // Validate the command fields.
  if (login.loginId.empty())
  {
    spdlog::debug(
      "LoginHandler::HandleUserLogin - Rejecting login for client {}."
      " User name or user token empty.",
      clientId);

    QueueUserLoginRejected(clientId);
    return;
  }

  // The login request must be unique for the client.
  if (_clientLogins.contains(clientId))
  {
    spdlog::debug(
      "LoginHandler::HandleUserLogin - Rejecting login for client {} ({})."
      " User login already queued.",
      clientId,
      login.loginId);

    QueueUserLoginRejected(clientId);
    return;
  }

  // Queue the login.
  const auto [iterator, inserted] =
    _clientLogins.try_emplace(clientId, LoginContext{
      .userName = login.loginId,
      .userToken = login.authKey});
  assert(inserted && "Duplicate client login request.");

  _clientLoginRequestQueue.emplace(clientId);
}

void LoginHandler::HandleUserCreateCharacter(
  ClientId clientId,
  const LobbyCommandCreateNickname& command)
{
  const auto& loginContext = _clientLogins[clientId];

  auto userRecord = _dataDirector.GetUsers().Get(loginContext.userName);
  if (not userRecord)
    throw std::runtime_error("User record does not exist");

  auto characterRecord = _dataDirector.CreateCharacter();
  soa::data::Uid userCharacterUid{soa::data::InvalidUid};

  characterRecord.Mutable([&userCharacterUid, &command](soa::data::Character& character) {
    userCharacterUid = character.uid();

    character.name = command.nickname;
    character.parts = soa::data::Character::Parts{
      .modelId = command.character.parts.charId,
      .mouthId = command.character.parts.mouthSerialId,
      .faceId = command.character.parts.faceSerialId};
    character.appearance = soa::data::Character::Appearance{
      .headSize = command.character.appearance.headSize,
      .height = command.character.appearance.height,
      .thighVolume = command.character.appearance.thighVolume,
      .legVolume = command.character.appearance.legVolume,};
  });

  userRecord->Mutable([&userCharacterUid](soa::data::User& user)
  {
    user.characterUid = userCharacterUid;
  });

  // Queue the processing of the login response.
  _clientLoginResponseQueue.emplace(clientId);
}

void LoginHandler::QueueUserLoginAccepted(
  const ClientId clientId,
  const std::string& userName)
{
  const auto lobbyServerTime = UnixTimeToFileTime(
  std::chrono::system_clock::now());

  LobbyCommandLoginOK response {
    .lobbyTime =
      {.dwLowDateTime = static_cast<uint32_t>(lobbyServerTime.dwLowDateTime),
       .dwHighDateTime = static_cast<uint32_t>(lobbyServerTime.dwHighDateTime)},
    .val0 = 0xCA794,

    .characterEquipment = {},
    .horseEquipment = {},

    .val1 = 0x0,
    .val2 = 0x0,
    .val3 = 0x0,

    .optionType = OptionType::Value,
    .valueOptions = 0x64,

    .val5 = {
      {0x18, {{2, 1}}},
      {0x1F, {{2, 1}}},
      {0x23, {{2, 1}}},
      {0x29, {{2, 1}}},
      {0x2A, {{2, 1}}},
      {0x2B, {{2, 1}}},
      {0x2E, {{2, 1}}}},

    .address = _lobbyDirector.GetSettings().ranchAdvAddress.to_uint(),
    .port = _lobbyDirector.GetSettings().ranchAdvPort,

    .scramblingConstant = 0,

    .val7 = {
      .values = {
        {0x6, 0x0},
          {0xF, 0x4},
          {0x1B, 0x2},
          {0x1E, 0x0},
          {0x1F, 0x0},
          {0x25, 0x7530},
          {0x35, 0x4},
          {0x42, 0x2},
          {0x43, 0x4},
          {0x45, 0x0}}},

    .val8 = 0b0000'0000'0000'0000'0000'0000'0000'0010,
    .val11 = {4, 0x2B, 4},
    .val14 = 0xca1b87db,
    .val15 = {.val1 = 1},
    .val16 = 4,
    .val17 = {.mountUid = 2, .val1 = 0x12, .val2 = 0x16e67e4},
    .val18 = 0x3a,
    .val19 = 0x38e,
    .val20 = 0x1c6};

  auto userRecord = _dataDirector.GetUsers().Get(userName);

  if (not userRecord)
    throw std::runtime_error("User record unavailable");

  // Get the character UID of the user.
  soa::data::Uid userCharacterUid{soa::data::InvalidUid};
  userRecord->Immutable([&userCharacterUid](
    const soa::data::User& user)
  {
    userCharacterUid = user.characterUid();
  });

  // Get the character record and fill the protocol data.
  // Also get the UID of the horse mounted by the character.
  auto characterRecord = _dataDirector.GetCharacters().Get(userCharacterUid);
  if (not characterRecord)
    throw std::runtime_error("Character record unavailable");

  soa::data::Uid characterMountUid{
    soa::data::InvalidUid};

  characterRecord->Immutable([&response, &characterMountUid](const soa::data::Character& character)
  {
    response.selfUid = character.uid();
    response.nickName = character.name();
    response.profileGender = Gender::Unspecified;

    response.level = character.level();
    response.carrots = character.carrots();

    response.ageGroup = AgeGroup::Adult;
    response.hideAge = false;

    // Set the character parts.
    // These serial ID's can be found in the `_ClientCharDefaultPartInfo` table.
    // Each character has specific part serial IDs for each part type.
    response.character.parts = {
      .charId = static_cast<uint8_t>(character.parts.modelId()),
      .mouthSerialId = static_cast<uint8_t>(character.parts.mouthId()),
      .faceSerialId = static_cast<uint8_t>(character.parts.faceId()),};

    // Set the character appearance.
    response.character.appearance = {
      .headSize = static_cast<uint8_t>(character.appearance.headSize()),
      .height = static_cast<uint8_t>(character.appearance.height()),
      .thighVolume = static_cast<uint8_t>(character.appearance.thighVolume()),
      .legVolume = static_cast<uint8_t>(character.appearance.legVolume()),};

    characterMountUid = character.mountUid();
  });

  // Get the mounted horse record and fill the protocol data.
  auto mountRecord = _dataDirector.GetHorses().Get(characterMountUid);
  if (not mountRecord)
    throw std::runtime_error("Morse mount record unavailable");

  mountRecord->Immutable([&response](const soa::data::Horse& horse)
  {
    response.horse = {
      .uid = horse.uid(),
      .tid = horse.tid(),
      .name = horse.name(),

      .rating = horse.rating(),
      .clazz = static_cast<uint8_t>(horse.clazz()),
      .val0 = 1,
      .grade = static_cast<uint8_t>(horse.grade()),
      .growthPoints = static_cast<uint16_t>(horse.growthPoints()),

      .vals0 = {
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
        .val10 = 0x00,},

      //
      .vals1 = {
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
        .emblem = static_cast<uint16_t>(horse.emblem())},

      .val16 = 0xb8a167e4,
      .val17 = 0};

    response.horse.parts = {
      .skinId = static_cast<uint8_t>(horse.parts.skinId()),
      .maneId = static_cast<uint8_t>(horse.parts.maneId()),
      .tailId = static_cast<uint8_t>(horse.parts.tailId()),
      .faceId = static_cast<uint8_t>(horse.parts.faceId())};

    response.horse.appearance = {
      .scale = static_cast<uint8_t>(horse.appearance.scale()),
      .legLength = static_cast<uint8_t>(horse.appearance.legLength()),
      .legVolume = static_cast<uint8_t>(horse.appearance.legVolume()),
      .bodyLength = static_cast<uint8_t>(horse.appearance.bodyLength()),
      .bodyVolume = static_cast<uint8_t>(horse.appearance.bodyVolume())};

    response.horse.stats = {
        .agility = horse.stats.agility(),
        .control = horse.stats.control(),
        .speed = horse.stats.speed(),
        .strength = horse.stats.strength(),
        .spirit = horse.stats.spirit()};

    response.horse.mastery = {
        .spurMagicCount = horse.mastery.spurMagicCount(),
        .jumpCount = horse.mastery.jumpCount(),
        .slidingTime = horse.mastery.slidingTime(),
        .glidingDistance = horse.mastery.glidingDistance(),};
  });

  _server.SetCode(clientId, {});

  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::LobbyLoginOK,
    [response]()
    {
      return response;
    });
}
void LoginHandler::QueueUserCreateNickname(ClientId clientId, const std::string& userName)
{
  _server.QueueCommand<LobbyCommandCreateNicknameNotify>(
    clientId,
    CommandId::LobbyCreateNicknameNotify,
    []()
    {
      return LobbyCommandCreateNicknameNotify{};
    });
}

void LoginHandler::QueueUserLoginRejected(ClientId clientId)
{
  _server.QueueCommand<LobbyCommandLoginCancel>(
    clientId,
    CommandId::LobbyLoginCancel,
    []()
    {
      return LobbyCommandLoginCancel{
        .reason = LoginCancelReason::InvalidUser};
    });
}

} // namespace alicia
