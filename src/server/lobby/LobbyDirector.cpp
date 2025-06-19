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

} // namespace

namespace server
{

LobbyDirector::LobbyDirector(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
  , _commandServer(*this)
  , _loginHandler(*this, _commandServer)
{
  _commandServer.RegisterCommandHandler<LobbyCommandLogin>(
    protocol::Command::LobbyLogin,
    [this](ClientId clientId, const auto& message)
  {
    assert(message.constant0 == 50 && message.constant1 == 281 && "Game version mismatch");

    _loginHandler.HandleUserLogin(clientId, message);
  });

  _commandServer.RegisterCommandHandler<LobbyCommandCreateNickname>(
    protocol::Command::LobbyCreateNickname,
    [this](ClientId clientId, const auto& message)
  {
    _loginHandler.HandleUserCreateCharacter(clientId, message);
  });

  _commandServer.RegisterCommandHandler<LobbyCommandEnterChannel>(
    protocol::Command::LobbyEnterChannel,
    [this](ClientId clientId, const auto& message)
  {
    HandleEnterChannel(clientId, message);
  });

  _commandServer.RegisterCommandHandler<LobbyCommandRoomList>(
    protocol::Command::LobbyRoomList,
    [this](ClientId clientId, const auto& message)
  {
    HandleRoomList(clientId, message);
  });

  _commandServer.RegisterCommandHandler<LobbyCommandMakeRoom>(
    protocol::Command::LobbyMakeRoom,
    [this](ClientId clientId, const auto& message)
  {
    HandleMakeRoom(clientId, message);
  });

  _commandServer.RegisterCommandHandler<LobbyCommandEnterRoom>(
    protocol::Command::LobbyEnterRoom,
    [this](ClientId clientId, const auto& message)
  {
    HandleEnterRoom(clientId, message);
  });

  _commandServer.RegisterCommandHandler<LobbyCommandHeartbeat>(
    protocol::Command::LobbyHeartbeat,
    [this](ClientId clientId, const auto& message)
  {
    HandleHeartbeat(clientId, message);
  });

  _commandServer.RegisterCommandHandler<LobbyCommandShowInventory>(
    protocol::Command::LobbyShowInventory,
    [this](ClientId clientId, const auto& message)
  {
    HandleShowInventory(clientId, message);
  });

  _commandServer.RegisterCommandHandler<LobbyCommandAchievementCompleteList>(
    protocol::Command::LobbyAchievementCompleteList,
    [this](ClientId clientId, const auto& message)
  {
    HandleAchievementCompleteList(clientId, message);
  });

  _commandServer.RegisterCommandHandler<LobbyCommandRequestLeagueInfo>(
    protocol::Command::LobbyRequestLeagueInfo,
    [this](ClientId clientId, const auto& message)
  {
    HandleRequestLeagueInfo(clientId, message);
  });

  _commandServer.RegisterCommandHandler<LobbyCommandRequestQuestList>(
    protocol::Command::LobbyRequestQuestList,
    [this](ClientId clientId, const auto& message)
  {
    HandleRequestQuestList(clientId, message);
  });

  _commandServer.RegisterCommandHandler<LobbyCommandRequestDailyQuestList>(
    protocol::Command::LobbyRequestDailyQuestList,
    [this](ClientId clientId, const auto& message)
  {
    HandleRequestDailyQuestList(clientId, message);
  });

  _commandServer.RegisterCommandHandler<LobbyCommandRequestSpecialEventList>(
    protocol::Command::LobbyRequestSpecialEventList,
    [this](ClientId clientId, const auto& message)
  {
    HandleRequestSpecialEventList(clientId, message);
  });

  _commandServer.RegisterCommandHandler<LobbyCommandRequestPersonalInfo>(
    protocol::Command::LobbyRequestPersonalInfo,
    [this](ClientId clientId, const auto& message)
  {
    HandleRequestPersonalInfo(clientId, message);
  });

  _commandServer.RegisterCommandHandler<LobbyCommandSetIntroduction>(
    protocol::Command::LobbySetIntroduction,
    [this](ClientId clientId, const auto& message)
  {
    HandleSetIntroduction(clientId, message);
  });

  _commandServer.RegisterCommandHandler<LobbyCommandEnterRanch>(
    protocol::Command::LobbyEnterRanch,
    [this](ClientId clientId, const auto& message)
  {
    HandleEnterRanch(clientId, message);
  });

  _commandServer.RegisterCommandHandler<LobbyCommandGetMessengerInfo>(
    protocol::Command::LobbyGetMessengerInfo,
    [this](ClientId clientId, const auto& message)
  {
    HandleGetMessengerInfo(clientId, message);
  });

  _commandServer.RegisterCommandHandler<LobbyCommandGoodsShopList>(
    protocol::Command::LobbyGoodsShopList,
    [this](ClientId clientId, const auto& message)
  {
    HandleGoodsShopList(clientId, message);
  });

  _commandServer.RegisterCommandHandler<LobbyCommandInquiryTreecash>(
    protocol::Command::LobbyInquiryTreecash,
    [this](ClientId clientId, const auto& message)
  {
    HandleInquiryTreecash(clientId, message);
  });

  _commandServer.RegisterCommandHandler<LobbyCommandClientNotify>(
    protocol::Command::LobbyClientNotify,
    [this](ClientId clientId, const auto& message)
  {
    if (message.val0 != 1)
      spdlog::error("Client error notification: state[{}], value[{}]", message.val0, message.val1);
  });

  _commandServer.RegisterCommandHandler<LobbyCommandEnterRandomRanch>(
    protocol::Command::LobbyEnterRandomRanch,
    [this](ClientId clientId, auto& command)
  {
    auto randomRanchUids = GetServerInstance().GetDataDirector().GetRanches().GetKeys();
    std::uniform_int_distribution<data::Uid> uidDistribution(
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

void LobbyDirector::HandleClientConnected(ClientId clientId)
{
  spdlog::info("Client {} connected to the lobby", clientId);
}

void LobbyDirector::HandleClientDisconnected(ClientId clientId)
{
  spdlog::info("Client {} disconnected from the lobby", clientId);
  _clientContext.erase(clientId);
}

ServerInstance& LobbyDirector::GetServerInstance()
{
  return _serverInstance;
}

Settings::LobbySettings& LobbyDirector::GetSettings()
{
  return GetServerInstance().GetSettings()._lobbySettings;
}

void LobbyDirector::UpdateInventory(uint32_t characterUid)
{
  for (const auto& [clientId, clientContext] : _clientContext)
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
    protocol::Command::LobbyEnterChannelOK,
    [response]()
  {
    return response;
  });
}

void LobbyDirector::HandleRoomList(
  ClientId clientId,
  const LobbyCommandRoomList& command)
{
  LobbyCommandRoomListOK response;
  response.page = command.page;
  response.unk1 = 1;
  response.unk2 = 1;

  for (const auto& room : RoomRegistry::Get().GetRooms() | std::views::values)
  {
    auto& roomResponse = response.rooms.emplace_back();
    roomResponse.id = room.uid;
    roomResponse.playerCount = 1;
    roomResponse.maxPlayers = 8;
    roomResponse.level = 2;
  }

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    protocol::Command::LobbyRoomListOK,
    [response]()
  {
    return response;
  });
}

void LobbyDirector::HandleMakeRoom(
  ClientId clientId,
  const LobbyCommandMakeRoom& command)
{
  auto& roomRegistry = RoomRegistry::Get();
  auto& room = roomRegistry.CreateRoom();

  room.name = command.name;
  room.description = command.password;
  room.missionId = command.missionId;
  room.unk0 = command.playerCount;
  room.gameMode = command.gameMode;
  room.teamMode = command.teamMode;
  room.unk3 = command.unk3;
  room.bitset = static_cast<uint16_t>(command.bitset);
  room.unk4 = command.unk4;

  LobbyCommandMakeRoomOK response{
    .roomUid = room.uid,
    .otp = 0xBAAD,
    .address = GetSettings().raceAdvAddress.to_uint(),
    .port = GetSettings().raceAdvPort};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    protocol::Command::LobbyMakeRoomOK,
    [response]()
  {
    return response;
  });
}

void LobbyDirector::HandleEnterRoom(
  ClientId clientId,
  const LobbyCommandEnterRoom& command)
{
  auto& roomRegistry = RoomRegistry::Get();

  LobbyCommandEnterRoomOK response{
    .roomUid = command.roomUid,
    .otp = 0xBAAD,
    .address = GetSettings().raceAdvAddress.to_uint(),
    .port = GetSettings().raceAdvPort,
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    protocol::Command::LobbyEnterRoomOK,
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

  characterRecord->Immutable(
    [this, &response](const data::Character& character)
  {
    const auto itemRecords = GetServerInstance().GetDataDirector().GetItems().Get(
      character.inventory());
    protocol::BuildProtocolItems(response.items, *itemRecords);

    const auto horseRecords = GetServerInstance().GetDataDirector().GetHorses().Get(
      character.horses());
    protocol::BuildProtocolHorses(response.horses, *horseRecords);
  });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    protocol::Command::LobbyShowInventoryOK,
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

  characterRecord->Immutable(
    [&response](const data::Character& character)
  {
    response.unk0 = character.uid();
  });

  response.achievements.emplace_back().tid = 20'008;
  response.achievements.emplace_back().tid = 20'004;

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    protocol::Command::LobbyAchievementCompleteListOK,
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
    protocol::Command::LobbyRequestLeagueInfoOK,
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

  characterRecord->Immutable(
    [&response](const data::Character& character)
  {
    response.unk0 = character.uid();
  });

  response.quests.emplace_back(Quest{.tid = 11'039, .member1 = 1, .member2 = 0xA, .member4 = 3});
  response.quests.emplace_back(Quest{.tid = 12'012, .member1 = 1, .member2 = 3, .member4 = 3});
  response.quests.emplace_back(Quest{.tid = 13'010, .member1 = 1, .member2 = 0x14, .member4 = 3});

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    protocol::Command::LobbyAchievementCompleteListOK,
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

  characterRecord->Immutable(
    [&response](const data::Character& character)
  {
    response.val0 = character.uid();
  });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    protocol::Command::LobbyRequestDailyQuestListOK,
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
    protocol::Command::LobbyAchievementCompleteListOK,
    [response]()
  {
    return response;
  });
}

void LobbyDirector::HandleRequestPersonalInfo(ClientId clientId, const LobbyCommandRequestPersonalInfo& command)
{
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    command.characterUid);

  LobbyCommandPersonalInfo response{
    .characterUid = command.characterUid,
    .type = command.type,
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    protocol::Command::LobbyPersonalInfo,
    [response]()
  {
    return response;
  });
}

void LobbyDirector::HandleSetIntroduction(ClientId clientId, const LobbyCommandSetIntroduction& command)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  characterRecord->Mutable(
    [&command](data::Character& character)
  {
    character.introduction() = command.introduction;
  });

  GetServerInstance().GetRanchDirector().BroadcastSetIntroductionNotify(
    clientContext.characterUid, command.introduction);
}

void LobbyDirector::HandleEnterRanch(
  ClientId clientId,
  const LobbyCommandEnterRanch& requestEnterRanch)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    requestEnterRanch.characterUid);

  if (not characterRecord)
  {
    LobbyCommandEnterRanchCancel response{};

    _commandServer.QueueCommand<decltype(response)>(
      clientId,
      protocol::Command::LobbyEnterRanchCancel,
      [response]()
    {
      return response;
    });
  }

  auto ranchUid = data::InvalidUid;
  characterRecord->Immutable(
    [&ranchUid](const data::Character& character)
  {
    ranchUid = character.ranchUid();
  });

  QueueEnterRanchOK(clientId, ranchUid);
}

void LobbyDirector::QueueEnterRanchOK(
  ClientId clientId,
  data::Uid ranchUid)
{
  LobbyCommandEnterRanchOK response{
    .ranchUid = ranchUid,
    .otp = 0x44332211,
    .ip = static_cast<uint32_t>(htonl(
      GetSettings().ranchAdvAddress.to_uint())),
    .port = GetSettings().ranchAdvPort};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    protocol::Command::LobbyEnterRanchOK,
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
    protocol::Command::LobbyGetMessengerInfoOK,
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
    protocol::Command::LobbyGoodsShopListOK,
    [response]()
  {
    return response;
  });
}

void LobbyDirector::HandleInquiryTreecash(
  ClientId clientId,
  const LobbyCommandInquiryTreecash& message)
{
  const auto& clientContext = _clientContext[clientId];
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  LobbyCommandInquiryTreecashOK response{};

  characterRecord->Immutable(
    [&response](const data::Character& character)
  {
    response.cash = character.cash();
  });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    protocol::Command::LobbyInquiryTreecashOK,
    [response]()
  {
    return response;
  });
}

void LobbyDirector::HandleGuildPartyList(
  ClientId clientId,
  const LobbyCommandGuildPartyList& message)
{
  const LobbyCommandGuildPartyListOK response{};
  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    protocol::Command::LobbyGuildPartyListOK,
    [response]()
  {
    return response;
  });
}

} // namespace server
