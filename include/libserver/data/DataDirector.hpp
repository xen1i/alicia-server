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

#include <list>
#include <queue>

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

  void RequestLoadUserData(const std::string& userName);
  void RequestUnloadUserData(const std::string& userName);
  bool IsUserDataLoaded(const std::string& userName);

  [[nodiscard]] Record<data::User> GetUser(const std::string& userName);
  [[nodiscard]] UserStorage& GetUsers();

  //! Gets a character.
  //! @param characterUid UID of the character.
  //! @returns The character record.
  [[nodiscard]] Record<data::Character> GetCharacter(data::Uid characterUid) noexcept;
  [[nodiscard]] Record<data::Character> CreateCharacter() noexcept;
  [[nodiscard]] CharacterStorage& GetCharacters();

  [[nodiscard]] Record<data::Pet> GetPet(data::Uid petUid) noexcept;
  [[nodiscard]] Record<data::Pet> CreatePet() noexcept;
  [[nodiscard]] PetStorage& GetPets();

  [[nodiscard]] Record<data::Guild> GetGuild(data::Uid guildUid) noexcept;
  [[nodiscard]] Record<data::Guild> CreateGuild() noexcept;
  [[nodiscard]] GuildStorage& GetGuilds();

  [[nodiscard]] Record<data::StoredItem> GetStoredItem(data::Uid storedItemUid) noexcept;
  [[nodiscard]] Record<data::StoredItem> CreateStoredItem() noexcept;
  [[nodiscard]] StoredItemStorage& GetStoredItems();

  [[nodiscard]] Record<data::Item> GetItem(data::Uid itemUid) noexcept;
  [[nodiscard]] Record<data::Item> CreateItem() noexcept;
  [[nodiscard]] ItemStorage& GetItems();

  [[nodiscard]] Record<data::Horse> GetHorse(data::Uid horseUid) noexcept;
  [[nodiscard]] Record<data::Horse> CreateHorse() noexcept;
  [[nodiscard]] HorseStorage& GetHorses();

  [[nodiscard]] Record<data::Ranch> GetRanch(data::Uid ranchUid) noexcept;
  [[nodiscard]] Record<data::Ranch> CreateRanch() noexcept;
  [[nodiscard]] RanchStorage& GetRanches();

private:
  //! An underlying data source of the data director.
  std::unique_ptr<FileDataSource> _primaryDataSource;

  using Clock = std::chrono::steady_clock;
  struct Job
  {
    Clock::time_point when;
    std::function<void()> job;
  };
  std::list<Job> _queuedJobs;

  struct UserDataContext
  {
    std::atomic_bool isBeingLoaded = false;
    std::atomic_bool isBeingUnloaded = false;
    std::atomic_bool isLoadedCompletely = false;
  };
  std::unordered_map<std::string, UserDataContext> _userDataContext;

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
