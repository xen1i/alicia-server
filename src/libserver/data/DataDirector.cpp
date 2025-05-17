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
        [&](auto& character) { _dataSource->StoreCharacter(character); },
        [&](auto& character) { _dataSource->RetrieveCharacter(character); })
    , _itemStorage(
        [&](auto& item) { _dataSource->StoreItem(item); },
        [&](auto& item) { _dataSource->RetrieveItem(item); })
    , _horseStorage(
        [&](auto& horse) { _dataSource->StoreHorse(horse); },
        [&](auto& horse) { _dataSource->RetrieveHorse(horse); })
    , _ranchStorage(
        [&](auto& ranch) { _dataSource->StoreRanch(ranch); },
        [&](auto& ranch) { _dataSource->RetrieveRanch(ranch); })
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

DataDirector::UserStorage& DataDirector::GetUsers()
{
  return _userStorage;
}

DataDirector::CharacterStorage& DataDirector::GetCharacters()
{
  return _characterStorage;
}

DataDirector::ItemStorage& DataDirector::GetItems()
{
  return _itemStorage;
}

DataDirector::HorseStorage& DataDirector::GetHorses()
{
  return _horseStorage;
}

DataDirector::RanchStorage& DataDirector::GetRanches()
{
  return _ranchStorage;
}

} // namespace soa
