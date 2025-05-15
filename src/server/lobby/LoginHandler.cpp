//
// Created by rgnter on 26/11/2024.
//

#include "server/lobby/LoginHandler.hpp"

#include "spdlog/spdlog.h"

namespace alicia
{

LoginHandler::LoginHandler(
  soa::DataDirector& dataDirector,
  CommandServer& server)
  : _server(server)
  , _dataDirector(dataDirector)
{
}

void LoginHandler::Tick()
{
  // Process the login queue.
  while (not _clientLoginRequestQueue.empty())
  {
    const ClientId clientId = _clientLoginRequestQueue.front();

    // Queued login must have a login context.
    assert(_clientLogins.contains(clientId));
    const auto& loginContext = _clientLogins[clientId];

    // Get the user credentials.
    auto& user = _dataDirector.GetUserStorage().Get(
      loginContext.userName);
    if (user.uid() == soa::data::InvalidUid)
    {
      continue;
    }

    _clientLoginRequestQueue.pop();

    // If the provided user token does not match the one stored
    // then reject the login.
    if (loginContext.userToken != user.token())
    {
      QueueUserLoginRejected(clientId);
    }
    else
    {
      // Queue the processing of the response.
      _clientLoginResponseQueue.emplace(clientId);
    }

    // Only one user login per tick.
    break;
  }

  while (not _clientLoginResponseQueue.empty())
  {
    const ClientId clientId = _clientLoginResponseQueue.front();

    assert(_clientLogins.contains(clientId));
    const auto& loginContext = _clientLogins[clientId];

    const auto& user = _dataDirector.GetUserStorage().Get(
      loginContext.userName);
    assert(user.uid() != soa::data::InvalidUid
      && "User must be available");

    // Load the character.
    auto character = _dataDirector.GetCharacter(user().characterUid);
    if (not character.IsAvailable())
    {
      continue;
    }

    const auto areDataAvailable = []<typename T>(const std::vector<T>& data)
    {
      for (const auto& datum : data)
      {
        if (not datum.IsAvailable())
        {
          return false;
        }
      }

      return true;
    };

    // Load the character equipment.
    const auto characterEquipment = _dataDirector.GetItems(
      character().characterEquipment);
    // Load the horse equipment.
    const auto horseEquipment = _dataDirector.GetItems(
      character().horseEquipment);
    // Load the horses.
    const auto horses = _dataDirector.GetHorses(
      character().horses);

    // Load the ranch.
    const auto ranch = _dataDirector.GetRanch(character().ranchUid);

    if (not areDataAvailable(characterEquipment) ||
      not areDataAvailable(horseEquipment) ||
      not areDataAvailable(horses) ||
      not ranch.IsAvailable())
    {
      continue;
    }

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

void LoginHandler::QueueUserLoginAccepted(
  const ClientId clientId,
  const std::string& userName)
{
  _server.QueueCommand<LobbyCommandLoginOK>(
    clientId,
    CommandId::LobbyLoginOK,
    [userName, this]()
    {
      // Get the user.
      const auto& user = _dataDirector.GetUserStorage().Get(userName);
      assert(user.uid() != soa::data::InvalidUid
        && "User must be available.");

      // Load the character.
      auto character = _dataDirector.GetCharacter(user().characterUid);
      assert(character.IsAvailable() && "Character must be available.");

      // Load the equipment.
      auto characterEquipment = _dataDirector.GetItems(
        character().characterEquipment);
      auto horseEquipment = _dataDirector.GetItems(
        character().horseEquipment);

      // Load the horses.
      auto horses = _dataDirector.GetHorses(character().horses);

      // Load the ranch
      auto ranch = _dataDirector.GetRanch(character().ranchUid);
      assert(ranch.IsAvailable());

      // Transform the server data to alicia protocol data.
      return LobbyCommandLoginOK{
      };
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


