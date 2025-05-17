//
// Created by alborrajo on 30/12/2024.
//

#ifndef RACEDIRECTOR_HPP
#define RACEDIRECTOR_HPP

#include "server/Settings.hpp"

#include "libserver/data/DataDirector.hpp"
#include "libserver/command/CommandServer.hpp"

namespace alicia
{

class RaceDirector
{
public:
  //!
  RaceDirector(
      soa::DataDirector& dataDirector,
      Settings::RaceSettings settings = {});

  void Initialize();
  void Terminate();
  void Tick();

private:

  //!
  Settings::RaceSettings _settings;
  //!
  soa::DataDirector& _dataDirector;
  //!
  CommandServer _server;

  //!
  std::unordered_map<ClientId, soa::data::Uid> _clientCharacters;

  struct RoomInstance
  {
    // Add race-specific data here
  };
  std::unordered_map<soa::data::Uid, RoomInstance> _rooms;
};

}

#endif //RACEDIRECTOR_HPP