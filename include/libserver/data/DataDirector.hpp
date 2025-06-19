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

  Record<data::Character> CreateCharacter();
  CharacterStorage& GetCharacters();

  Record<data::Item> CreateItem();
  ItemStorage& GetItems();

  Record<data::Pet> CreatePet();
  PetStorage& GetPets();

  Record<data::Guild> CreateGuild();
  GuildStorage& GetGuilds();

  Record<data::StoredItem> CreateStoredItem();
  StoredItemStorage& GetStoredItems();

  Record<data::Horse> CreateHorse();
  HorseStorage& GetHorses();

  Record<data::Ranch> CreateRanch();
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
