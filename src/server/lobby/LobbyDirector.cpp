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

#include "server/lobby/LobbyDirector.hpp"

#include "libserver/data/helper/ProtocolHelper.hpp"
#include "libserver/registry/RoomRegistry.hpp"
#include "server/ServerInstance.hpp"

#include <random>

#include <spdlog/spdlog.h>

namespace
{

std::random_device rd;

} // anon namespace

namespace alicia
{

LobbyDirector::LobbyDirector(soa::ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
  , _loginHandler(*this, _commandServer)
{
  _commandServer.RegisterCommandHandler<LobbyCommandLogin>(
    CommandId::LobbyLogin,
    [this](ClientId clientId, const auto& message)
    {
      assert(message.constant0 == 50 && message.constant1 == 281 && "Game version mismatch");

      _loginHandler.HandleUserLogin(clientId, message);
    });

  _commandServer.RegisterCommandHandler<LobbyCommandCreateNickname>(
    CommandId::LobbyCreateNickname,
    [this](ClientId clientId, const auto& message)
    {
      _loginHandler.HandleUserCreateCharacter(clientId, message);
    });

  _commandServer.RegisterCommandHandler<LobbyCommandEnterChannel>(
    CommandId::LobbyEnterChannel,
    [this](ClientId clientId, const auto& message)
    {
      HandleEnterChannel(clientId, message);
    });

  _commandServer.RegisterCommandHandler<LobbyCommandMakeRoom>(
    CommandId::LobbyMakeRoom,
    [this](ClientId clientId, const auto& message)
    {
      HandleMakeRoom(clientId, message);
    });

  _commandServer.RegisterCommandHandler<LobbyCommandHeartbeat>(
    CommandId::LobbyHeartbeat,
    [this](ClientId clientId, const auto& message)
    {
      HandleHeartbeat(clientId, message);
    });

  _commandServer.RegisterCommandHandler<LobbyCommandShowInventory>(
    CommandId::LobbyShowInventory,
    [this](ClientId clientId, const auto& message)
    {
      HandleShowInventory(clientId, message);
    });

  _commandServer.RegisterCommandHandler<LobbyCommandAchievementCompleteList>(
    CommandId::LobbyAchievementCompleteList,
    [this](ClientId clientId, const auto& message)
    {
      HandleAchievementCompleteList(clientId, message);
    });

  _commandServer.RegisterCommandHandler<LobbyCommandRequestLeagueInfo>(
    CommandId::LobbyRequestLeagueInfo,
    [this](ClientId clientId, const auto& message)
    {
      HandleRequestLeagueInfo(clientId, message);
    });

  _commandServer.RegisterCommandHandler<LobbyCommandRequestQuestList>(
    CommandId::LobbyRequestQuestList,
    [this](ClientId clientId, const auto& message)
    {
      HandleRequestQuestList(clientId, message);
    });

  _commandServer.RegisterCommandHandler<LobbyCommandRequestDailyQuestList>(
    CommandId::LobbyRequestDailyQuestList,
    [this](ClientId clientId, const auto& message)
    {
      HandleRequestDailyQuestList(clientId, message);
    });

  _commandServer.RegisterCommandHandler<LobbyCommandRequestSpecialEventList>(
    CommandId::LobbyRequestSpecialEventList,
    [this](ClientId clientId, const auto& message)
    {
      HandleRequestSpecialEventList(clientId, message);
    });

  _commandServer.RegisterCommandHandler<LobbyCommandEnterRanch>(
    CommandId::LobbyEnterRanch,
    [this](ClientId clientId, const auto& message)
    {
      HandleEnterRanch(clientId, message);
    });

  _commandServer.RegisterCommandHandler<LobbyCommandGetMessengerInfo>(
    CommandId::LobbyGetMessengerInfo,
    [this](ClientId clientId, const auto& message)
    {
      HandleGetMessengerInfo(clientId, message);
    });

  _commandServer.RegisterCommandHandler<LobbyCommandGoodsShopList>(
    CommandId::LobbyGoodsShopList,
    [this](ClientId clientId, const auto& message)
    {
      HandleGoodsShopList(clientId, message);
    });

  _commandServer.RegisterCommandHandler<LobbyCommandInquiryTreecash>(
    CommandId::LobbyInquiryTreecash,
    [this](ClientId clientId, const auto& message)
    {
      HandleInquiryTreecash(clientId, message);
    });

  _commandServer.RegisterCommandHandler<LobbyCommandClientNotify>(
    CommandId::LobbyClientNotify,
    [this](ClientId clientId, const auto& message)
    {
      if (message.val0 != 1)
        spdlog::error("Client error notification: state[{}], value[{}]", message.val0, message.val1);
    });

  _commandServer.RegisterCommandHandler<LobbyCommandEnterRandomRanch>(
    CommandId::LobbyEnterRandomRanch,
    [this](ClientId clientId, auto& command)
    {
      const auto randomRanchUids = GetServerInstance().GetDataDirector().GetRanches().GetKeys();
      std::uniform_int_distribution<soa::data::Uid> uidDistribution(
        0, randomRanchUids.size() - 1);

      QueueEnterRanchOK(clientId, randomRanchUids[uidDistribution(rd)]);
    });
}

void LobbyDirector::Initialize()
{
  spdlog::debug(
    "Lobby is advertising ranch server on {}:{}",
    GetSettings().ranchAdvAddress.to_string(),
    GetSettings().ranchAdvPort);
  spdlog::debug(
    "Lobby is advertising messenger server on {}:{}",
    GetSettings().messengerAdvAddress.to_string(),
    GetSettings().messengerAdvPort);

  spdlog::debug(
    "Lobby server listening on {}:{}",
    GetSettings().address.to_string(),
    GetSettings().port);

  _commandServer.BeginHost(GetSettings().address, GetSettings().port);
}

void LobbyDirector::Terminate()
{
  _commandServer.EndHost();
}

void LobbyDirector::Tick()
{
  _loginHandler.Tick();
}

soa::ServerInstance& LobbyDirector::GetServerInstance()
{
  return _serverInstance;
}

soa::Settings::LobbySettings& LobbyDirector::GetSettings()
{
  return GetServerInstance().GetSettings()._lobbySettings;
}

void LobbyDirector::UpdateInventory(uint32_t characterUid)
{
  for (const auto & [clientId, clientContext] : _clientContext)
  {
    if (clientContext.characterUid != characterUid)
      continue;

    QueueShowInventory(clientId);
    break;
  }
}

void LobbyDirector::HandleEnterChannel(
  ClientId clientId,
  const LobbyCommandEnterChannel& enterChannel)
{
  LobbyCommandEnterChannelOK response{
    .unk0 = enterChannel.channel // potentially
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::LobbyEnterChannelOK,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleMakeRoom(
  ClientId clientId,
  const LobbyCommandMakeRoom& makeRoom)
{
  auto& roomRegistry = soa::RoomRegistry::Get();
  auto& room = roomRegistry.CreateRoom();

  room.name = makeRoom.name;
  room.description = makeRoom.description;
  room.missionId = makeRoom.missionId;
  room.unk0 = makeRoom.unk0;
  room.unk1 = makeRoom.unk1;
  room.unk2 = makeRoom.unk2;
  room.unk3 = makeRoom.unk3;
  room.bitset = makeRoom.bitset;
  room.unk4 = makeRoom.unk4;

  LobbyCommandMakeRoomOK response{
    .roomUid = room.uid,
    .otp = 0xBAAD,
    .ip = htonl(GetSettings().raceAdvAddress.to_uint()),
    .port = GetSettings().raceAdvPort};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::LobbyMakeRoomOK,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleHeartbeat(
  ClientId clientId,
  const LobbyCommandHeartbeat& heartbeat)
{
  // TODO: Statistics for the heartbeat.
}

void LobbyDirector::HandleShowInventory(
  ClientId clientId,
  const LobbyCommandShowInventory& showInventory)
{
  QueueShowInventory(clientId);
}

void LobbyDirector::QueueShowInventory(ClientId clientId)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  LobbyCommandShowInventoryOK response{
    .items = {},
    .horses = {}};

  characterRecord->Immutable([this, &response](const soa::data::Character& character)
  {
    auto itemRecords = GetServerInstance().GetDataDirector().GetItems().Get(character.inventory());
    protocol::BuildProtocolItems(response.items, *itemRecords);

    auto horseRecords = GetServerInstance().GetDataDirector().GetHorses().Get(character.horses());
    protocol::BuildProtocolHorses(response.horses, *horseRecords);
  });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::LobbyShowInventoryOK,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleAchievementCompleteList(
  ClientId clientId,
  const LobbyCommandAchievementCompleteList& achievementCompleteList)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  LobbyCommandAchievementCompleteListOK response{};

  characterRecord->Immutable([&response](const soa::data::Character& character)
  {
    response.unk0 = character.uid();
  });

  response.achievements.emplace_back().tid = 20'008;
  response.achievements.emplace_back().tid = 20'004;

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::LobbyAchievementCompleteListOK,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleRequestLeagueInfo(
  ClientId clientId,
  const LobbyCommandRequestLeagueInfo& requestLeagueInfo)
{
  LobbyCommandRequestLeagueInfoOK response{};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::LobbyRequestLeagueInfoOK,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleRequestQuestList(
  ClientId clientId,
  const LobbyCommandRequestQuestList& requestQuestList)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  LobbyCommandRequestQuestListOK response{};

  characterRecord->Immutable([&response](const soa::data::Character& character)
  {
    response.unk0 = character.uid();
  });

  response.quests.emplace_back(Quest{
    .tid = 11'039,
    .member1 = 1,
    .member2 = 0xA,
    .member4 = 3});
  response.quests.emplace_back(Quest{
    .tid = 12'012,
    .member1 = 1,
    .member2 = 3,
    .member4 = 3});
  response.quests.emplace_back(Quest{
    .tid = 13'010,
    .member1 = 1,
    .member2 = 0x14,
    .member4 = 3});

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::LobbyAchievementCompleteListOK,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleRequestDailyQuestList(
  ClientId clientId,
  const LobbyCommandRequestDailyQuestList& requestQuestList)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  LobbyCommandRequestDailyQuestListOK response{};

  characterRecord->Immutable([&response](const soa::data::Character& character)
  {
    response.val0 = character.uid();
  });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::LobbyRequestDailyQuestListOK,
    [response]()
    {
      return response;
    });
}
void LobbyDirector::HandleRequestSpecialEventList(
  ClientId clientId,
  const LobbyCommandRequestSpecialEventList& requestQuestList)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  LobbyCommandRequestSpecialEventListOK response{};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::LobbyAchievementCompleteListOK,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleEnterRanch(
  ClientId clientId,
  const LobbyCommandEnterRanch& requestEnterRanch)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  if (not characterRecord)
  {
    throw std::runtime_error(
      std::format("Character [{}] not available", clientContext.characterUid));
  }

  auto ranchUid = soa::data::InvalidUid;
  characterRecord->Immutable([&ranchUid](const soa::data::Character& character)
  {
    ranchUid = character.ranchUid();
  });

  QueueEnterRanchOK(clientId, ranchUid);
}

void LobbyDirector::QueueEnterRanchOK(
  ClientId clientId,
  soa::data::Uid ranchUid)
{
  LobbyCommandEnterRanchOK response{
    .ranchUid =  ranchUid,
    .code = 0x44332211,
    .ip = static_cast<uint32_t>(htonl(GetSettings().ranchAdvAddress.to_uint())),
    .port = GetSettings().ranchAdvPort};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::LobbyEnterRanchOK,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleGetMessengerInfo(
  ClientId clientId,
  const LobbyCommandGetMessengerInfo& requestMessengerInfo)
{
  LobbyCommandGetMessengerInfoOK response{
    .code = 0xDEAD,
    .ip = static_cast<uint32_t>(htonl(GetSettings().messengerAdvAddress.to_uint())),
    .port = GetSettings().messengerAdvPort,
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::LobbyGetMessengerInfoOK,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleGoodsShopList(
  ClientId clientId,
  const LobbyCommandGoodsShopList& message)
{
  LobbyCommandGoodsShopListOK response{
    .data = message.data};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::LobbyGoodsShopListOK,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleInquiryTreecash(
  ClientId clientId,
  const LobbyCommandInquiryTreecash& message)
{
  LobbyCommandInquiryTreecashOK response{
    .cash = 1'000};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::LobbyInquiryTreecashOK,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleGuildPartyList(
  ClientId clientId,
  const LobbyCommandGuildPartyList& message)
{
}

//
// void LobbyDirector::HandleGuildPartyList(
//   ClientId clientId,
//   const LobbyCommandGuildPartyList& message)
// {
//   const LobbyCommandGuildPartyListOK response{};
//
//   _server.QueueCommand(
//     clientId,
//     CommandId::LobbyGuildPartyListOK,
//     [response](SinkStream& sink)
//     {
//       LobbyCommandGuildPartyListOK::Write(response, sink);
//     });
// }

void LobbyCommandEnterRandomRanch::Write(
  const LobbyCommandEnterRandomRanch& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void LobbyCommandEnterRandomRanch::Read(
  LobbyCommandEnterRandomRanch& command,
  SourceStream& stream)
{
  // Empty.
}

} // namespace alicia
