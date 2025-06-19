/**
 * Alicia Server - dedicated server software
 * Copyright (C) 2024 Story Of Alicia
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 **/

#include "libserver/data/DataDirector.hpp"

namespace server
{

DataDirector::DataDirector()
  : _userStorage(
      [&](const auto& key, auto& user)
      {
        _dataSource->RetrieveUser(key, user);
      },
      [&](const auto& key, auto& user)
      {
        _dataSource->StoreUser(key, user);
      })
  , _characterStorage(
      [&](const auto& key, auto& character)
      {
        _dataSource->RetrieveCharacter(key, character);
      },
      [&](const auto& key, auto& character)
      {
        _dataSource->StoreCharacter(key, character);
      })
  , _itemStorage(
      [&](const auto& key, auto& item)
      {
        _dataSource->RetrieveItem(key, item);
      },
      [&](const auto& key, auto& item)
      {
        _dataSource->StoreItem(key, item);
      })
  , _storedItemStorage(
      [&](const auto& key, auto& item)
      {
        _dataSource->RetrieveStoredItem(key, item);
      },
      [&](const auto& key, auto& item)
      {
        _dataSource->StoreStoredItem(key, item);
      })
  , _petStorage(
      [&](const auto& key, auto& item)
      {
        _dataSource->RetrievePet(key, item);
      },
      [&](const auto& key, auto& item)
      {
        _dataSource->StorePet(key, item);
      })
  , _guildStorage(
      [&](const auto& key, auto& item)
      {
        _dataSource->RetrieveGuild(key, item);
      },
      [&](const auto& key, auto& item)
      {
        _dataSource->StoreGuild(key, item);
      })
  , _horseStorage(
      [&](const auto& key, auto& horse)
      {
        _dataSource->RetrieveHorse(key, horse);
      },
      [&](const auto& key, auto& horse)
      {
        _dataSource->StoreHorse(key, horse);
      })
  , _ranchStorage(
      [&](const auto& key, auto& ranch)
      {
        _dataSource->RetrieveRanch(key, ranch);
      },
      [&](const auto& key, auto& ranch)
      {
        _dataSource->StoreRanch(key, ranch);
      })
{
  _dataSource = std::make_unique<FileDataSource>();
  _dataSource->Initialize("./data");
}

DataDirector::~DataDirector()
{
  _dataSource->Terminate();
}

void DataDirector::Initialize()
{
}

void DataDirector::Terminate()
{
  _userStorage.Terminate();
  _characterStorage.Terminate();
  _itemStorage.Terminate();
  _horseStorage.Terminate();
  _ranchStorage.Terminate();
}

void DataDirector::Tick()
{
  _userStorage.Tick();
  _characterStorage.Tick();
  _itemStorage.Tick();
  _horseStorage.Tick();
  _ranchStorage.Tick();
}

DataDirector::UserStorage& DataDirector::GetUsers()
{
  return _userStorage;
}

Record<data::Character> DataDirector::CreateCharacter()
{
  return _characterStorage.Create(
    [this]()
    {
      data::Character character;
      _dataSource->CreateCharacter(character);

      return std::make_pair(character.uid(), std::move(character));
    });
}

DataDirector::CharacterStorage& DataDirector::GetCharacters()
{
  return _characterStorage;
}

Record<data::Item> DataDirector::CreateItem()
{
  return _itemStorage.Create(
    [this]()
    {
      data::Item item;
      _dataSource->CreateItem(item);

      return std::make_pair(item.uid(), std::move(item));
    });
}

DataDirector::ItemStorage& DataDirector::GetItems()
{
  return _itemStorage;
}

Record<data::Pet> DataDirector::CreatePet()
{
  return _petStorage.Create(
    [this]()
    {
      data::Pet pet;
      _dataSource->CreatePet(pet);

      return std::make_pair(pet.uid(), std::move(pet));
    });
}

DataDirector::PetStorage& DataDirector::GetPets()
{
  return _petStorage;
}

Record<data::Guild> DataDirector::CreateGuild()
{
  return _guildStorage.Create(
    [this]()
    {
      data::Guild guild;
      _dataSource->CreateGuild(guild);

      return std::make_pair(guild.uid(), std::move(guild));
    });
}

DataDirector::GuildStorage& DataDirector::GetGuilds()
{
  return _guildStorage;
}

Record<data::StoredItem> DataDirector::CreateStoredItem()
{
  return _storedItemStorage.Create(
    [this]()
    {
      data::StoredItem item;
      _dataSource->CreateStoredItem(item);

      return std::make_pair(item.uid(), std::move(item));
    });
}

DataDirector::StoredItemStorage& DataDirector::GetStoredItems()
{
  return _storedItemStorage;
}

Record<data::Horse> DataDirector::CreateHorse()
{
  return _horseStorage.Create(
    [this]()
    {
      data::Horse horse;
      _dataSource->CreateHorse(horse);

      return std::make_pair(horse.uid(), std::move(horse));
    });
}

DataDirector::HorseStorage& DataDirector::GetHorses()
{
  return _horseStorage;
}

Record<data::Ranch> DataDirector::CreateRanch()
{
  return _ranchStorage.Create(
    [this]()
    {
      data::Ranch ranch;
      _dataSource->CreateRanch(ranch);

      return std::make_pair(ranch.uid(), std::move(ranch));
    });
}

DataDirector::RanchStorage& DataDirector::GetRanches()
{
  return _ranchStorage;
}

} // namespace server
