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
        try
        {
          _primaryDataSource->RetrieveUser(key, user);
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception retrieving user '{}' from the primary data source: {}",
            key,
            x.what());
        }
      },
      [&](const auto& key, auto& user)
      {
        try
        {
          _primaryDataSource->StoreUser(key, user);
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception storing user '{}' on the primary data source: {}", key, x.what());
        }
      })
  , _characterStorage(
      [&](const auto& key, auto& character)
      {
        try
        {
          _primaryDataSource->RetrieveCharacter(key, character);
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception retrieving character {} from the primary data source: {}", key, x.what());
        }
      },
      [&](const auto& key, auto& character)
      {
        try
        {
          _primaryDataSource->StoreCharacter(key, character);
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception storing character {} on the primary data source: {}", key, x.what());
        }
      })
  , _itemStorage(
      [&](const auto& key, auto& item)
      {
        try
        {
          _primaryDataSource->RetrieveItem(key, item);
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception retrieving item {} from the primary data source: {}", key, x.what());
        }
      },
      [&](const auto& key, auto& item)
      {
        try
        {
          _primaryDataSource->StoreItem(key, item);
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception storing item {} on the primary data source: {}", key, x.what());
        }
      })
  , _storedItemStorage(
      [&](const auto& key, auto& storedItem)
      {
        try
        {
          _primaryDataSource->RetrieveStoredItem(key, storedItem);
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception retrieving stored item {} from the primary data source: {}", key, x.what());
        }
      },
      [&](const auto& key, auto& storedItem)
      {
        try
        {
          _primaryDataSource->StoreStoredItem(key, storedItem);
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception storing stored item {} on the primary data source: {}", key, x.what());
        }
      })
  , _petStorage(
      [&](const auto& key, auto& pet)
      {
        try
        {
          _primaryDataSource->RetrievePet(key, pet);
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception retrieving pet {} from the primary data source: {}", key, x.what());
        }
      },
      [&](const auto& key, auto& pet)
      {
        try
        {
          _primaryDataSource->StorePet(key, pet);
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception storing pet {} on the primary data source: {}", key, x.what());
        }
      })
  , _guildStorage(
      [&](const auto& key, auto& guild)
      {
        try
        {
          _primaryDataSource->RetrieveGuild(key, guild);
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception retrieving guild {} from the primary data source: {}", key, x.what());
        }
      },
      [&](const auto& key, auto& guild)
      {
        try
        {
          _primaryDataSource->StoreGuild(key, guild);
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception storing guild {} on the primary data source: {}", key, x.what());
        }
      })
  , _horseStorage(
      [&](const auto& key, auto& horse)
      {
        try
        {
          _primaryDataSource->RetrieveHorse(key, horse);
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception retrieving horse {} from the primary data source: {}", key, x.what());
        }
      },
      [&](const auto& key, auto& horse)
      {
        try
        {
          _primaryDataSource->StoreHorse(key, horse);
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception storing horse {} on the primary data source: {}", key, x.what());
        }
      })
  , _ranchStorage(
      [&](const auto& key, auto& ranch)
      {
        try
        {
          _primaryDataSource->RetrieveRanch(key, ranch);
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception retrieving ranch {} from the primary data source: {}", key, x.what());
        }
      },
      [&](const auto& key, auto& ranch)
      {
        try
        {
          _primaryDataSource->StoreRanch(key, ranch);
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception storing ranch {} on the primary data source: {}", key, x.what());
        }
      })
{
  _primaryDataSource = std::make_unique<FileDataSource>();
  _primaryDataSource->Initialize("./data");
}

DataDirector::~DataDirector()
{
  _primaryDataSource->Terminate();
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
      _primaryDataSource->CreateCharacter(character);

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
      _primaryDataSource->CreateItem(item);

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
      _primaryDataSource->CreatePet(pet);

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
      _primaryDataSource->CreateGuild(guild);

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
      _primaryDataSource->CreateStoredItem(item);

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
      _primaryDataSource->CreateHorse(horse);

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
      _primaryDataSource->CreateRanch(ranch);

      return std::make_pair(ranch.uid(), std::move(ranch));
    });
}

DataDirector::RanchStorage& DataDirector::GetRanches()
{
  return _ranchStorage;
}

} // namespace server
