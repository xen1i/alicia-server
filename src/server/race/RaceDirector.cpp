//
// Created by alborrajo on 30/12/2024.
//

#include "server/race/RaceDirector.hpp"
#include "spdlog/spdlog.h"

namespace alicia
{

RaceDirector::RaceDirector(soa::DataDirector& dataDirector, Settings::RaceSettings settings)
    : _settings(std::move(settings))
    , _dataDirector(dataDirector)
    , _server()
{

}

void RaceDirector::Initialize()
{
  spdlog::debug(
    "Race server listening on {}:{}",
    _settings.address.to_string(),
    _settings.port);

  // Host the server
  _server.Host(_settings.address, _settings.port);
}

void RaceDirector::Terminate()
{
}

void RaceDirector::Tick() {}

} // namespace alicia