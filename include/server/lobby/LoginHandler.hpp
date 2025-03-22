//
// Created by rgnter on 26/11/2024.
//

#ifndef LOGINHANDLER_HPP
#define LOGINHANDLER_HPP

#include "server/DataDirector.hpp"
#include "libserver/command/CommandServer.hpp"

namespace alicia
{

//! Login handler.
class LoginHandler
{
public:
  LoginHandler(
    DataDirector& dataDirector,
    CommandServer& server);

  void Tick();

  //!
  void HandleUserLogin(
    ClientId clientId,
    const LobbyCommandLogin& login);

  void QueueUserLoginAccepted(ClientId clientId, const DatumUid& characterUid);
  void QueueUserLoginRejected(ClientId clientId);

private:
  struct LoginContext
  {
    ClientId clientId;
    std::string userName;
    std::string userToken;

    Record<data::User>::View user;
  };

  std::unordered_map<ClientId, LoginContext> _clientLogins;
  std::queue<ClientId> _clientLoginRequestQueue;
  std::queue<ClientId> _clientLoginResponseQueue;

  //!
  CommandServer& _server;
  //!
  DataDirector& _dataDirector;
};

} // namespace alicia

#endif //LOGINHANDLER_HPP
