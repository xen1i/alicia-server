//
// Created by maros on 4/7/25.
//

#ifndef DATADIRECTOR_HPP
#define DATADIRECTOR_HPP

#include "DataDefinitions.hpp"
#include "DataSource.hpp"
#include "DataStorage.hpp"

#include <queue>

namespace soa
{

class DataDirector
{
public:
  void Tick();

  bool SynchronizeUser(const std::string& username) noexcept
  {
    return false;
  }

  bool IsUserSynchronized(const std::string& username) noexcept
  {
    auto* userRecord = _users.Get(username);
    return userRecord != nullptr;
  }

  data::User& GetUser(const std::string& username)
  {
    auto* userRecord = _users.Get(username);
    if (userRecord == nullptr)
      throw std::runtime_error("User not available");

    return *userRecord;
  }

private:
  DataSource _dataSource;
  DataStorage<std::string, data::User> _users;
};

}

#endif //DATADIRECTOR_HPP
