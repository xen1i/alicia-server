//
// Created by rgnter on 30/05/2025.
//

#ifndef DIRECTOR_HPP
#define DIRECTOR_HPP

#include "libserver/data/DataDirector.hpp"
#include "libserver/network/command/CommandServer.hpp"

namespace alicia
{

class BaseDirector
{
public:
  struct ClientContext
  {
    bool authorized{false};
    soa::data::Uid _characterUid{soa::data::InvalidUid};
  };

  using ClientContextMap = std::unordered_map<ClientId, ClientContext>;

  BaseDirector(soa::DataDirector& _dataDirector);
  virtual ~BaseDirector() = default;

private:
  soa::DataDirector& _dataDirector;
  CommandServer _commandServer;
  ClientContextMap _clientContext;
};

}

#endif //DIRECTOR_HPP
