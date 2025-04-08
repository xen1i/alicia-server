//
// Created by maros on 4/7/25.
//

#include "libserver/data/DataDirector.hpp"

namespace soa
{

DataDirector::DataDirector(const std::string& url)
  : _userStorage(
    [this](data::User& user)
    {
      _dataSource.RetrieveUser(user);
    },
    [this](const data::User& user)
    {
      _dataSource.StoreUser(user);
    })
{
  _dataSource.Establish(url);
}

void DataDirector::Tick()
{
  if (not _dataSource.IsConnectionFine())
  {
    spdlog::error("Data source connection dropped.");
  }

  _userStorage.Tick();
}

bool DataDirector::IsUserAvailable(const std::string& name)
{
  return _userStorage.IsAvailable(name);
}

data::User& DataDirector::GetUser(const std::string& name)
{
  return _userStorage.Get(name);
}

} // namespace soa
