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

#ifndef DATADIRECTOR_HPP
#define DATADIRECTOR_HPP

#include "DataDefinitions.hpp"
#include "DataStorage.hpp"

// #include "pq/PqDataSource.hpp"
#include "file/FileDataSource.hpp"

namespace server
{

class DataDirector
{
public:
  using UserStorage = DataStorage<std::string, data::User>;
  using CharacterStorage = DataStorage<data::Uid, data::Character>;
  using ItemStorage = DataStorage<data::Uid, data::Item>;
  using PetStorage = DataStorage<data::Uid, data::Pet>;
  using GuildStorage = DataStorage<data::Uid, data::Guild>;
  using StoredItemStorage = DataStorage<data::Uid, data::StoredItem>;
  using HorseStorage = DataStorage<data::Uid, data::Horse>;
  using RanchStorage = DataStorage<data::Uid, data::Ranch>;

  //! Default constructor.
  explicit DataDirector();
  ~DataDirector();

  void Initialize();
  void Terminate();

  //! Ticks the data director.
  void Tick();

  UserStorage& GetUsers();

  Record<data::Character> CreateCharacter()
  {
    return _characterStorage.Create([this]() {
      data::Character character;
      _dataSource->CreateCharacter(character);

      return std::make_pair(character.uid(), std::move(character));
    });
  }
  CharacterStorage& GetCharacters();

  Record<data::Item> CreateItem()
  {
    return _itemStorage.Create([this]() {
      data::Item item;
      _dataSource->CreateItem(item);

      return std::make_pair(item.uid(), std::move(item));
    });
  }
  ItemStorage& GetItems();

  Record<data::Pet> CreatePet()
  {
    return _petStorage.Create([this]() {
      data::Pet pet;
      _dataSource->CreatePet(pet);

      return std::make_pair(pet.uid(), std::move(pet));
    });
  }
  PetStorage& GetPets();

  Record<data::Guild> CreateGuild()
  {
    return _guildStorage.Create([this]() {
      data::Guild guild;
      _dataSource->CreateGuild(guild);

      return std::make_pair(guild.uid(), std::move(guild));
    });
  }
  GuildStorage& GetGuilds();

  Record<data::StoredItem> CreateStoredItem()
  {
    return _storedItemStorage.Create([this]() {
      data::StoredItem item;
      _dataSource->CreateStoredItem(item);

      return std::make_pair(item.uid(), std::move(item));
    });
  }
  StoredItemStorage& GetStoredItems();

  Record<data::Horse> CreateHorse()
  {
    return _horseStorage.Create([this]() {
      data::Horse horse;
      _dataSource->CreateHorse(horse);

      return std::make_pair(horse.uid(), std::move(horse));
    });
  }
  HorseStorage& GetHorses();

  Record<data::Ranch> CreateRanch()
  {
    return _ranchStorage.Create([this]() {
      data::Ranch ranch;
      _dataSource->CreateRanch(ranch);

      return std::make_pair(ranch.uid(), std::move(ranch));
    });
  }
  RanchStorage& GetRanches();

private:
  //! An underlying data source of the data director.
  std::unique_ptr<FileDataSource> _dataSource;

  //! A user storage.
  UserStorage _userStorage;
  //! A character storage.
  CharacterStorage _characterStorage;
  //! A item storage.
  ItemStorage _itemStorage;
  //! A stored item storage.
  StoredItemStorage _storedItemStorage;
  //! A pet storage.
  PetStorage _petStorage;
  //! A guild storage.
  GuildStorage _guildStorage;
  //! A horse storage.
  HorseStorage _horseStorage;
  //! A ranch storage.
  RanchStorage _ranchStorage;
};

} // namespace server

#endif // DATADIRECTOR_HPP
