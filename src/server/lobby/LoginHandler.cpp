//
// Created by rgnter on 26/11/2024.
//

#include "server/lobby/LoginHandler.hpp"

#include "spdlog/spdlog.h"

namespace alicia
{

LoginHandler::LoginHandler(DataDirector& dataDirector, CommandServer& server)
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

    assert(_clientLogins.contains(clientId));
    const LoginContext& loginContext = _clientLogins[clientId];

    // Get the user credentials.
    const auto user = _dataDirector.GetUser(loginContext.userName);
    if (not user)
    {
      continue;
    }

    _clientLoginRequestQueue.pop();

    // If the provided user token does not match the one stored
    // then reject the login.
    if (loginContext.userToken != user.token)
    {
      QueueUserLoginRejected(clientId);
    }
    else
    {
      // Queue the response.
      _clientLoginResponseQueue.emplace(clientId);
    }

    // Only one user login per tick.
    break;
  }

  while (not _clientLoginResponseQueue.empty())
  {
    const ClientId clientId = _clientLoginResponseQueue.front();

    assert(_clientLogins.contains(clientId));
    const LoginContext& loginContext = _clientLogins[clientId];

    // Load the character.
    const auto character = _dataDirector.GetCharacter(user.characterUid);
    if (not character)
    {
      continue;
    }

    // Load the equipment.
    const auto characterEquipment = _dataDirector.GetItems(character.characterEquipment);
    const auto horseEquipment = _dataDirector.GetItems(character.characterEquipment);
    // Load the horses.
    const auto horses = _dataDirector.GetHorses(character.horses);
    // Load the ranch
    const auto ranch = _dataDirector.GetRanch(character.ranchUid);

    if (not characterEquipment ||
      not horseEquipment ||
      not horses ||
      not ranch)
    {
      continue;
    }

    QueueUserLoginAccepted(clientId, user.characterUid);
  }
}

void LoginHandler::HandleUserLogin(
  const ClientId clientId,
  const LobbyCommandLogin& login)
{
  if (login.loginId.empty() || login.authKey.empty())
  {
    spdlog::debug(
      "LoginHandler::HandleUserLogin - Rejecting login for client {}."
      " User name or user token empty.",
      clientId);

    QueueUserLoginRejected(clientId);
    return;
  }

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

  const auto [iterator, inserted] =
    _clientLogins.try_emplace(clientId, LoginContext{
      .userName = login.loginId,
      .userToken = login.authKey});
  assert(not inserted && "Duplicate client login request.");

  _clientLoginRequestQueue.emplace(clientId);
}

void LoginHandler::QueueUserLoginAccepted(
  const ClientId clientId,
  const data::User& user)
{
  _server.QueueCommand<LobbyCommandLoginOK>(
    clientId,
    CommandId::LobbyLoginOK,
    [&user, this]()
    {
      // Load the character.
      const auto character = _dataDirector.GetCharacter(user.characterUid);
      assert(character.IsAvailable());

      // Load the equipment.
      const auto characterEquipment = _dataDirector.GetItems(character.characterEquipment);
      const auto horseEquipment = _dataDirector.GetItems(character.characterEquipment);
      assert(characterEquipment.IsAvailable() && horseEquipment.IsAvailable());

      // Load the horses.
      const auto horses = _dataDirector.GetHorses(character.horses);
      assert(horses.IsAvailable());

      // Load the ranch
      const auto ranch = _dataDirector.GetRanch(character.ranchUid);
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


