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

namespace soa
{

class DataDirector
{
public:
  using UserStorage = DataStorage<std::string, data::User>;
  using CharacterStorage = DataStorage<data::Uid, data::Character>;
  using ItemStorage = DataStorage<data::Uid, data::Item>;
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

  Record<data::Horse> CreateHorse()
  {
    return _horseStorage.Create([this]() {
      data::Horse horse;
      _dataSource->CreateHorse(horse);

      return std::make_pair(horse.uid(), std::move(horse));
    });
  }

  ItemStorage& GetItems();

  Record<data::Item> CreateItem()
  {
    return _itemStorage.Create([this]() {
      data::Item item;
      _dataSource->CreateItem(item);

      return std::make_pair(item.uid(), std::move(item));
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
  std::unique_ptr<FileDataSource> _dataSource;

  UserStorage _userStorage;
  CharacterStorage _characterStorage;
  HorseStorage _horseStorage;
  RanchStorage _ranchStorage;
  ItemStorage _itemStorage;
};

} // namespace soa

#endif // DATADIRECTOR_HPP
