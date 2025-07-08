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

#ifndef FILEDATASOURCE_HPP
#define FILEDATASOURCE_HPP

#include <libserver/data/DataDefinitions.hpp>
#include <libserver/data/DataSource.hpp>

namespace server
{

class FileDataSource
  : public DataSource
{
public:
  void Initialize(const std::filesystem::path& path);
  void Terminate();

  void RetrieveUser(std::string name, data::User& user) override;
  void StoreUser(std::string name, const data::User& user) override;

  void CreateCharacter(data::Character& character) override;
  void RetrieveCharacter(data::Uid uid, data::Character& character) override;
  void StoreCharacter(data::Uid uid, const data::Character& character) override;

  void CreateItem(data::Item& item) override;
  void RetrieveItem(data::Uid uid, data::Item& item) override;
  void StoreItem(data::Uid uid, const data::Item& item) override;

  void CreateStoredItem(data::StoredItem& item) override;
  void RetrieveStoredItem(data::Uid uid, data::StoredItem& item) override;
  void StoreStoredItem(data::Uid uid, const data::StoredItem& item) override;

  void CreatePet(data::Pet& pet) override;
  void RetrievePet(data::Uid uid, data::Pet& pet) override;
  void StorePet(data::Uid uid, const data::Pet& pet) override;

  void CreateGuild(data::Guild& guild) override;
  void RetrieveGuild(data::Uid uid, data::Guild& guild) override;
  void StoreGuild(data::Uid uid, const data::Guild& guild) override;

  void CreateHorse(data::Horse& horse) override;
  void RetrieveHorse(data::Uid uid, data::Horse& horse) override;
  void StoreHorse(data::Uid uid, const data::Horse& horse) override;

private:
  std::filesystem::path _dataPath;

  //! A path to the user data files.
  std::filesystem::path _userDataPath;
  //! A path to the character data files.
  std::filesystem::path _characterDataPath;
  //! A path to the item data files.
  std::filesystem::path _itemDataPath;
  //! A path to the pet data files.
  std::filesystem::path _petDataPath;
  //! A path to the guild data files.
  std::filesystem::path _guildDataPath;
  //! A path to the stored item data files.
  std::filesystem::path _storedItemPath;
  //! A path to the horse data files.
  std::filesystem::path _horseDataPath;
  //! A path to the ranch data files.
  std::filesystem::path _ranchDataPath;

  std::filesystem::path _metaFilePath;

  std::atomic_uint32_t _sequentialUid = 0;
};

} // namespace server

#endif // FILEDATASOURCE_HPP
