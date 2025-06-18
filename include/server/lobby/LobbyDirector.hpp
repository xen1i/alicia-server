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

#ifndef LOBBYDIRECTOR_HPP
#define LOBBYDIRECTOR_HPP

#include "LoginHandler.hpp"

#include "libserver/network/command/CommandServer.hpp"
#include "libserver/network/command/proto/LobbyMessageDefinitions.hpp"

namespace soa
{
class ServerInstance;
} // namespace soa

namespace alicia
{

class LobbyDirector final
  : public CommandServer::EventInterface
{
  friend LoginHandler;

public:
  //!
  explicit LobbyDirector(soa::ServerInstance& serverInstance);

  LobbyDirector(const LobbyDirector&) = delete;
  LobbyDirector& operator=(const LobbyDirector&) = delete;

  LobbyDirector(LobbyDirector&&) = delete;
  LobbyDirector& operator=(LobbyDirector&&) = delete;

  void Initialize();
  void Terminate();
  void Tick();

  void HandleClientConnected(ClientId clientId) override;
  void HandleClientDisconnected(ClientId client) override;

  soa::ServerInstance& GetServerInstance();
  soa::Settings::LobbySettings& GetSettings();

  void UpdateInventory(uint32_t characterUid);

private:
  //!
  void HandleEnterChannel(
    ClientId clientId,
    const LobbyCommandEnterChannel& enterChannel);

  void HandleRoomList(
    ClientId,
    const LobbyCommandRoomList& command);

  //!
  void HandleMakeRoom(
    ClientId clientId,
    const LobbyCommandMakeRoom& makeRoom);

  //!
  void HandleHeartbeat(
    ClientId clientId,
    const LobbyCommandHeartbeat& heartbeat);

  //!
  void HandleShowInventory(
    ClientId clientId,
    const LobbyCommandShowInventory& showInventory);

  void QueueShowInventory(
    ClientId clientId);

  //!
  void HandleAchievementCompleteList(
    ClientId clientId,
    const LobbyCommandAchievementCompleteList& achievementCompleteList);

  //!
  void HandleRequestLeagueInfo(
    ClientId clientId,
    const LobbyCommandRequestLeagueInfo& requestLeagueInfo);

  //!
  void HandleRequestQuestList(
    ClientId clientId,
    const LobbyCommandRequestQuestList& requestQuestList);

  //!
  void HandleRequestDailyQuestList(
    ClientId clientId,
    const LobbyCommandRequestDailyQuestList& requestQuestList);

  //!
  void HandleRequestSpecialEventList(
    ClientId clientId,
    const LobbyCommandRequestSpecialEventList& requestQuestList);

  //!
  void HandleRequestPersonalInfo(
    ClientId clientId,
    const LobbyCommandRequestPersonalInfo& command);

  void HandleSetIntroduction(
    ClientId clientId,
    const LobbyCommandSetIntroduction& command);

  //!
  void HandleEnterRanch(
    ClientId clientId,
    const LobbyCommandEnterRanch& requestEnterRanch);

  void QueueEnterRanchOK(
    ClientId clientId,
    soa::data::Uid ranchUid);

  //!
  void HandleGetMessengerInfo(
    ClientId clientId,
    const LobbyCommandGetMessengerInfo& requestMessengerInfo);

  //!
  void HandleGoodsShopList(
    ClientId clientId,
    const LobbyCommandGoodsShopList& message);

  //!
  void HandleInquiryTreecash(
    ClientId clientId,
    const LobbyCommandInquiryTreecash& message);

  //!
  void HandleGuildPartyList(
    ClientId clientId,
    const LobbyCommandGuildPartyList& message);

  //!
  soa::ServerInstance& _serverInstance;
  //!
  CommandServer _commandServer;
  //!
  LoginHandler _loginHandler;

protected:
  struct ClientContext
  {
    bool authorized;
    soa::data::Uid characterUid = soa::data::InvalidUid;
  };
  
  //!
  std::unordered_map<ClientId, ClientContext> _clientContext;
};

} // namespace alicia

#endif // LOBBYDIRECTOR_HPP
