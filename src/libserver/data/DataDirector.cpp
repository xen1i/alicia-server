//
// Created by maros on 4/7/25.
//

#include "libserver/data/DataDirector.hpp"

namespace soa
{

DataDirector::DataDirector()
    : _userStorage(
        [&](auto& user) { _dataSource->StoreUser(user); },
        [&](auto& user) { _dataSource->RetrieveUser(user); })
    , _characterStorage(
        [&](auto& user) { _dataSource->StoreCharacter(user); },
        [&](auto& user) { _dataSource->RetrieveCharacter(user); })
{
  _dataSource = std::make_unique<FileDataSource>();
  _dataSource->Initialize("./");
}

DataDirector::~DataDirector()
{
  _dataSource->Terminate();
}

void DataDirector::Tick()
{
  _userStorage.Tick();
  _characterStorage.Tick();
}

DataDirector::UserStorage& DataDirector::GetUserStorage()
{
  return _userStorage;
}

DataDirector::CharacterStorage& DataDirector::GetCharacterStorage()
{
  return _characterStorage;
}

} // namespace soa
