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

#include "server/race/RaceDirector.hpp"

#include "libserver/data/helper/ProtocolHelper.hpp"
#include "server/ServerInstance.hpp"

#include "libserver/registry/RoomRegistry.hpp"

#include <spdlog/spdlog.h>

namespace alicia
{

RaceDirector::RaceDirector(soa::ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
  , _commandServer(*this)
{
  _commandServer.RegisterCommandHandler<RaceCommandEnterRoom>(
    CommandId::RaceEnterRoom,
    [this](ClientId clientId, const auto& message)
    {
      HandleEnterRoom(clientId, message);
    });

  _commandServer.RegisterCommandHandler<RaceCommandChangeRoomOptions>(
    CommandId::RaceChangeRoomOptions,
    [this](ClientId clientId, const auto& message)
    {
      HandleChangeRoomOptions(clientId, message);
    });

  _commandServer.RegisterCommandHandler<RaceCommandStartRace>(
    CommandId::RaceStartRace,
    [this](ClientId clientId, const auto& message)
    {
      HandleStartRace(clientId, message);
    });

  _commandServer.RegisterCommandHandler<UserRaceTimer>(
    CommandId::UserRaceTimer,
    [this](ClientId clientId, const auto& message)
    {
      HandleRaceTimer(clientId, message);
    });

  _commandServer.RegisterCommandHandler<RaceCommandLoadingComplete>(
    CommandId::RaceLoadingComplete,
    [this](ClientId clientId, const auto& message)
    {
      _commandServer.QueueCommand<RaceCommandLoadingCompleteNotify>(clientId, CommandId::RaceLoadingCompleteNotify, [](){
        return RaceCommandLoadingCompleteNotify{
        .member0 = 1};
      });
    });

  _commandServer.RegisterCommandHandler<RaceCommandReadyRace>(
    CommandId::RaceReady,
    [this](ClientId clientId, const auto& message)
    {
      HandleReadyRace(clientId, message);
    });
}

void RaceDirector::Initialize()
{
  spdlog::debug(
    "Race server listening on {}:{}",
    GetSettings().address.to_string(),
    GetSettings().port);

  // Host the server
  _commandServer.BeginHost(GetSettings().address, GetSettings().port);
}

void RaceDirector::Terminate()
{
  _commandServer.EndHost();
}

void RaceDirector::Tick() {}

void RaceDirector::HandleClientDisconnected(ClientId client)
{
}

void RaceDirector::HandleClientConnected(ClientId clientId)
{
}

soa::ServerInstance& RaceDirector::GetServerInstance()
{
  return _serverInstance;
}

soa::Settings::RaceSettings& RaceDirector::GetSettings()
{
  return GetServerInstance().GetSettings()._raceSettings;
}

/*

Rooms will not be in data director.

  struct Room
  {
    std::string name;
    std::string description;
    uint8_t unk0;
    uint8_t unk1;
    uint8_t isBusy;
    uint16_t missionId;
    uint8_t unk4;
    uint16_t bitset;
    uint8_t unk6;
  };

*/

void RaceDirector::HandleEnterRoom(ClientId clientId, const RaceCommandEnterRoom& enterRoom)
{
  assert(enterRoom.otp == 0xBAAD);

  // TODO: Send RaceEnterRoomNotify to all clients in the room

  // Todo: Roll the code for the connecting client.
  // Todo: The response contains the code, somewhere.
  _commandServer.SetCode(clientId, {});

  auto& roomRegistry = soa::RoomRegistry::Get();
  auto& room = roomRegistry.GetRoom(enterRoom.roomUid);

  RaceCommandEnterRoomOK response{
    .nowPlaying = 1,
    .unk1 = 1,
    .roomDescription = {
      .name = room.name,
      .val_between_name_and_desc = static_cast<uint8_t>(room.uid), // ?
      .description = room.description,
      .unk1 = room.unk0,
      .unk2 = room.unk1,
      .unk3 = 8,
      .unk4 = room.unk2,
      .missionId = room.missionId,
      .unk6 = room.unk3,
      .unk7 = room.unk4
    }
  };

  auto& racer = response.racers.emplace_back();

  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(enterRoom.characterUid);
  characterRecord->Immutable([this, &racer](const soa::data::Character& character)
  {
    racer.uid = character.uid();
    racer.name = character.name();

    racer.playerRacer = PlayerRacer{
    };

    protocol::BuildProtocolCharacter(racer.playerRacer->character, character);

    const auto mountRecord = GetServerInstance().GetDataDirector().GetHorses().Get(character.mountUid());
    mountRecord->Immutable([&racer](const soa::data::Horse& mount)
    {
      protocol::BuildProtocolHorse(racer.playerRacer->horse, mount);
    });
  });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RaceEnterRoomOK,
    [response]()
    {
      return response;
    });
}

void RaceDirector::HandleChangeRoomOptions(ClientId clientId, const RaceCommandChangeRoomOptions& changeRoomOptions)
{
  // TODO: Actually do something

  RaceCommandChangeRoomOptionsNotify response{
    .optionsBitfield = changeRoomOptions.optionsBitfield,
    .option0 = changeRoomOptions.name,
    .option1 = changeRoomOptions.val_between_name_and_desc,
    .option2 = changeRoomOptions.description,
    .option3 = changeRoomOptions.option3,
    .option4 = changeRoomOptions.map,
    .option5 = changeRoomOptions.raceStarted};

  // TODO: Send to all clients in the room
  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RaceChangeRoomOptionsNotify,
    [response]()
    {
      return response;
    });
}

void RaceDirector::HandleStartRace(ClientId clientId, const RaceCommandStartRace& startRace)
{
  // Start the race or AcCmdRCRoomCountdown
  const RaceCommandStartRaceNotify response {
    .gamemode = 6,
    .unk3 = 1,
    .map = 8,
    .racers = {
      {
        .oid = 1,
        .name = "regent",
        .unk2 = 1,
        .unk3 = 1,
        .unk4 = 1,
        .p2dId = 3,
        .unk6 = 1,
        .unk7 = 3,
      }
    },
    .ip = GetSettings().address.to_uint(),
    .port = htons(GetSettings().port),
  };

  // TODO: Send to all clients in the room
  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RaceStartRaceNotify,
    [response]()
    {
      return response;
    });
}

void RaceDirector::HandleRaceTimer(ClientId clientId, const UserRaceTimer& raceTimer)
{
  UserRaceTimerOK response{
    .unk0 = raceTimer.timestamp + 10000,
    .unk1 = raceTimer.timestamp + 20000};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::UserRaceTimerOK,
    [response]()
    {
      return response;
    });
}

void RaceDirector::HandleReadyRace(ClientId clientId, const RaceCommandReadyRace& command)
{
  RaceCommandReadyRaceNotify response{
    .characterUid = 3,
    .ready = true};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RaceReadyNotify,
    [response]()
    {
      return response;
    });

  HandleStartRace(clientId, {});
}

} // namespace alicia
