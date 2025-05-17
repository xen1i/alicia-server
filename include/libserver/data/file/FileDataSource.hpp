//
// Created by rgnter on 14/05/2025.
//

#ifndef FILEDATASOURCE_HPP
#define FILEDATASOURCE_HPP

#include <libserver/data/DataDefinitions.hpp>
#include <libserver/data/DataSource.hpp>

#include <pqxx/pqxx>

namespace soa
{

class FileDataSource
  : public DataSource
{
public:
  void Initialize(const std::filesystem::path& path);
  void Terminate();

  void RetrieveUser(data::User& user) override;

  void StoreUser(const data::User& user) override;

  void RetrieveCharacter(data::Character& character) override;

  void StoreCharacter(const data::Character& character) override;

  void RetrieveItem(data::Item& item) override;

  void StoreItem(const data::Item& item) override;

  void RetrieveHorse(data::Horse& horse) override;

  void StoreHorse(const data::Horse& horse) override;

  void RetrieveRanch(data::Ranch& ranch) override;

  void StoreRanch(const data::Ranch& ranch) override;

private:
  std::filesystem::path _path;
  std::filesystem::path _usersPath;
  std::filesystem::path _charactersPath;
  std::filesystem::path _metaFilePath;

  uint32_t _sequentialUid = 0;
};

}

#endif //FILEDATASOURCE_HPP
