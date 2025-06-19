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

#ifndef RACEDIRECTOR_HPP
#define RACEDIRECTOR_HPP

#include "server/Settings.hpp"

#include "libserver/network/command/CommandServer.hpp"
#include "libserver/network/command/proto/RaceMessageDefinitions.hpp"
#include "server/tracker/WorldTracker.hpp"

namespace soa
{
class ServerInstance;
} // namespace soa

namespace alicia
{

class RaceDirector final
  : public CommandServer::EventHandlerInterface
{
public:
  //!
  explicit RaceDirector(soa::ServerInstance& serverInstance);

  void Initialize();
  void Terminate();
  void Tick();

  void HandleClientConnected(ClientId clientId) override;
  void HandleClientDisconnected(ClientId clientId) override;

  soa::ServerInstance& GetServerInstance();
  soa::Settings::RaceSettings& GetSettings();

private:
  //!
  void HandleEnterRoom(
    ClientId clientId,
    const RaceCommandEnterRoom& enterRoom);

  //!
  void HandleChangeRoomOptions(
    ClientId clientId,
    const RaceCommandChangeRoomOptions& changeRoomOptions);

  //!
  void HandleStartRace(
    ClientId clientId,
    const RaceCommandStartRace& startRace);

  //!
  void HandleRaceTimer(
    ClientId clientId,
    const UserRaceTimer& raceTimer);

  void HandleReadyRace(
    ClientId clientId,
    const RaceCommandReadyRace& command);

  //!
  soa::ServerInstance& _serverInstance;
  //!
  CommandServer _commandServer;

  struct ClientContext
  {
    uint32_t characterUid;
    uint32_t roomUid;
    bool ready = false;
  };
  std::unordered_map<ClientId, ClientContext> _clientContexts;

  struct Room
  {
    std::vector<ClientId> clients;
    WorldTracker worldTracker;
  };
  std::unordered_map<uint32_t, Room> _roomInstances;
};

} // namespace alicia

#endif // RACEDIRECTOR_HPP
