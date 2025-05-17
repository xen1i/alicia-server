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
    const auto& loginContext = _clientLogins[clientId];

    // Load the user.
    auto user = _dataDirector.GetUsers().Get(
      loginContext.userName);
    if (not user)
    {
      continue;
    }

    _clientLoginRequestQueue.pop();

    bool isAuthenticated = false;
    user->Immutable([&isAuthenticated, &loginContext](auto& user)
    {
      isAuthenticated = user.token() == loginContext.userToken;
    });

    // If the user succeeds in authentication queue user for further processing.
    if (isAuthenticated)
    {
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

    // Load the character.

    auto user = *_dataDirector.GetUsers().Get(loginContext.userName);
    auto character = _dataDirector.GetCharacters().Get(user().characterUid());
    if (not character)
    {
      continue;
    }

    // Load the character's equipment, horses and ranch.

    const auto characterEquipment = _dataDirector.GetItems().Get(
      (*character)().characterEquipment());
    const auto horseEquipment = _dataDirector.GetItems().Get(
      (*character)().horseEquipment());

    const auto horses = _dataDirector.GetHorses().Get((*character)().horses());

    const auto ranch = _dataDirector.GetRanches().Get((*character)().ranchUid());

    if (not characterEquipment
      || not horseEquipment
      || not horses
      || not ranch)
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
      auto user = *_dataDirector.GetUsers().Get(userName);
      auto character = *_dataDirector.GetCharacters().Get(user().characterUid());

      auto characterEquipment = *_dataDirector.GetItems().Get(
        character().characterEquipment());
      auto horseEquipment = *_dataDirector.GetItems().Get(
        character().horseEquipment());

      auto horses = *_dataDirector.GetHorses().Get(character().horses());

      auto ranch = *_dataDirector.GetRanches().Get(character().ranchUid());


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


