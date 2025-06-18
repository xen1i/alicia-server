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
#include "server/ServerInstance.hpp"

#include "libserver/data/helper/ProtocolHelper.hpp"

#include "spdlog/spdlog.h"

namespace alicia
{

LoginHandler::LoginHandler(
  LobbyDirector& lobbyDirector,
  CommandServer& server)
  : _server(server)
  , _lobbyDirector(lobbyDirector)
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
    auto user = _lobbyDirector.GetServerInstance().GetDataDirector().GetUsers().Get(loginContext.userName);
    if (not user)
      continue;

    _clientLoginRequestQueue.pop();

    bool isAuthenticated = false;
    bool hasCharacter = false;
    user->Immutable([&isAuthenticated, &hasCharacter, &loginContext](const soa::data::User& user){
      isAuthenticated = user.token() == loginContext.userToken;
      hasCharacter = user.characterUid() != soa::data::InvalidUid;
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

    // Preload all the user data.

    auto userRecord = _lobbyDirector.GetServerInstance().GetDataDirector().GetUsers().Get(
      loginContext.userName);
    if (not userRecord)
      continue;

    auto characterUid = soa::data::InvalidUid;
    userRecord->Immutable([&characterUid](const soa::data::User& user)
    {
      characterUid = user.characterUid();
    });

    auto characterRecord = _lobbyDirector.GetServerInstance().GetDataDirector().GetCharacters().Get(
      characterUid);
    if (not characterRecord)
      continue;

    bool isCharacterLoaded = false;

    characterRecord->Immutable([this, &isCharacterLoaded](const soa::data::Character& character)
    {
      if (not _lobbyDirector.GetServerInstance().GetDataDirector().GetItems().Get(character.inventory()))
        return;
      if (not _lobbyDirector.GetServerInstance().GetDataDirector().GetItems().Get(character.characterEquipment()))
        return;
      if (not _lobbyDirector.GetServerInstance().GetDataDirector().GetItems().Get(character.mountEquipment()))
        return;

      if (not _lobbyDirector.GetServerInstance().GetDataDirector().GetHorses().Get(character.horses()))
        return;

      if (not _lobbyDirector.GetServerInstance().GetDataDirector().GetRanches().Get(character.ranchUid()))
        return;

      isCharacterLoaded = true;
    });

    if (not isCharacterLoaded)
      continue;

    _clientLoginResponseQueue.pop();

    _lobbyDirector._clientContext[clientId] = {
      .authorized = true,
      .characterUid = characterUid};

    QueueUserLoginAccepted(clientId, loginContext.userName);
  }
}

void LoginHandler::HandleUserLogin(
  const ClientId clientId,
  const LobbyCommandLogin& login)
{
  // Validate the command fields.
  if (login.loginId.empty() || login.authKey.empty())
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

  auto userRecord = _lobbyDirector.GetServerInstance().GetDataDirector().GetUsers().Get(loginContext.userName);
  if (not userRecord)
    throw std::runtime_error("User record does not exist");

  // Create a new horse for the character.
  auto horseRecord = _lobbyDirector.GetServerInstance().GetDataDirector().CreateHorse();
  // The UID of the newly created horse.
  auto characterMountUid{soa::data::InvalidUid};

  horseRecord.Mutable([&characterMountUid](soa::data::Horse& horse)
  {
    horse.name() = "default";

    // The TID of the horse specifies which body mesh is used for that horse.
    // Can be found in the `MountPartInfo` table.
    horse.tid() = 20002;

    // The default parts and appearance of each horse TID
    // can be found in the `MountPartSet` table.
    horse.parts.skinId() = 4;
    horse.parts.faceId() = 1;
    horse.parts.maneId() = 1;
    horse.parts.tailId() = 1;
    horse.appearance.bodyLength() = 5;
    horse.appearance.bodyVolume() = 5;
    horse.appearance.legLength() = 5;
    horse.appearance.legVolume() = 5;
    horse.appearance.scale() = 5;

    characterMountUid = horse.uid();
  });

  // Create a new ranch for the character.
  auto ranchRecord = _lobbyDirector.GetServerInstance().GetDataDirector().CreateRanch();
  // The UID of the newly created ranch.
  auto characterRanchUid{soa::data::InvalidUid};

  ranchRecord.Mutable([&characterRanchUid, &command](soa::data::Ranch& ranch)
  {
    const bool endsWithPlural = command.nickname.ends_with("s")
      || command.nickname.ends_with("S");
    const std::string possessiveSuffix = endsWithPlural ? "'" : "'s";

    ranch.name = std::format("{}{} ranch", command.nickname, possessiveSuffix);
    characterRanchUid = ranch.uid();
  });

  // And finally create the character with the new horse,
  // new ranch and the appearance sent from the client.
  auto characterRecord = _lobbyDirector.GetServerInstance().GetDataDirector().CreateCharacter();
  auto userCharacterUid{soa::data::InvalidUid};

  characterRecord.Mutable([
    &userCharacterUid,
    &characterMountUid,
    &characterRanchUid,
    &command](soa::data::Character& character)
  {
    userCharacterUid = character.uid();

    character.level = 60;
    character.carrots = 5000;

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

    character.mountUid() = characterMountUid;
    character.horses().emplace_back(characterMountUid);

    character.ranchUid() = characterRanchUid;
  });

  // Assign the character to the user.
  userRecord->Mutable([&userCharacterUid](soa::data::User& user)
  {
    user.characterUid() = userCharacterUid;
  });

  // Queue the processing of the login response.
  _clientLoginResponseQueue.emplace(clientId);
}

void LoginHandler::QueueUserLoginAccepted(
  const ClientId clientId,
  const std::string& userName)
{
  auto userRecord = _lobbyDirector.GetServerInstance().GetDataDirector().GetUsers().Get(userName);
  if (not userRecord)
    throw std::runtime_error("User record unavailable");

  const auto lobbyServerTime = soa::util::UnixTimeToFileTime(
    std::chrono::system_clock::now());

  LobbyCommandLoginOK response {
    .lobbyTime =
      {.dwLowDateTime = static_cast<uint32_t>(lobbyServerTime.dwLowDateTime),
       .dwHighDateTime = static_cast<uint32_t>(lobbyServerTime.dwHighDateTime)},
    .member0 = 0xCA794,
    .motd = std::format(
      "Welcome to Story of Alicia. Players online: {}",
      _lobbyDirector._clientContext.size()),
    .val1 = 0x0,
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

    .val11 = {4, 0x2B, 4},
    .val14 = 0xca1b87db,
    .guild = {.val1 = 1},
    .val16 = 4,
    .val18 = 0x3a,
    .val19 = 0x38e,
    .val20 = 0x1c6};

  // Get the character UID of the user.
  soa::data::Uid userCharacterUid{soa::data::InvalidUid};
  userRecord->Immutable([&userCharacterUid](
    const soa::data::User& user)
  {
    userCharacterUid = user.characterUid();
  });

  // Get the character record and fill the protocol data.
  // Also get the UID of the horse mounted by the character.
  auto characterRecord = _lobbyDirector.GetServerInstance().GetDataDirector().GetCharacters().Get(userCharacterUid);
  if (not characterRecord)
    throw std::runtime_error("Character record unavailable");

  soa::data::Uid characterMountUid{
    soa::data::InvalidUid};

  characterRecord->Immutable([this, &response, &characterMountUid](const soa::data::Character& character)
  {
    response.uid = character.uid();
    response.name = character.name();

    response.introduction = character.introduction();
    response.gender = Gender::Unspecified;

    response.level = character.level();
    response.carrots = character.carrots();
    response.role = std::bit_cast<LobbyCommandLoginOK::Role>(
      character.role());
    response.ageGroup = AgeGroup::Adult;
    response.hideAge = false;

    response.bitfield = 0x0e06;

    // Character equipment.
    auto characterEquipmentItems = _lobbyDirector.GetServerInstance().GetDataDirector().GetItems().Get(
      character.characterEquipment());
    if (not characterEquipmentItems)
      throw std::runtime_error("Character equipment items unavailable");

    protocol::BuildProtocolItems(
      response.characterEquipment,
      *characterEquipmentItems);

    // Mount equipment.
    auto mountEquipmentItems = _lobbyDirector.GetServerInstance().GetDataDirector().GetItems().Get(
      character.mountEquipment());
    if (not mountEquipmentItems)
      throw std::runtime_error("Character equipment items unavailable");

    protocol::BuildProtocolItems(
      response.mountEquipment,
      *mountEquipmentItems);

    protocol::BuildProtocolCharacter(
      response.character,
      character);

    characterMountUid = character.mountUid();
  });

  // Get the mounted horse record and fill the protocol data.
  auto mountRecord = _lobbyDirector.GetServerInstance().GetDataDirector().GetHorses().Get(characterMountUid);
  if (not mountRecord)
    throw std::runtime_error("Horse mount record unavailable");

  mountRecord->Immutable([&response](const soa::data::Horse& horse)
  {
//    response.val17 = {
//      .mountUid = horse.uid(),
//      .tid = 0x12,
//      .val2 = 0x16e67e4};

    protocol::BuildProtocolHorse(response.horse, horse);
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
