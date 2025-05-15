//
// Created by maros on 4/7/25.
//

#ifndef DATADIRECTOR_HPP
#define DATADIRECTOR_HPP

#include "DataDefinitions.hpp"
#include "DataStorage.hpp"

#include "pq/PqDataSource.hpp"
#include "file/FileDataSource.hpp"

namespace soa
{

class DataDirector
{
public:
  using UserStorage = DataStorage<std::string, data::User>;
  using CharacterStorage = DataStorage<std::string, data::Character>;

  //! Default constructor.
  //! @param url pq url
  explicit DataDirector();
  ~DataDirector();

  //! Ticks the data director.
  void Tick();

  UserStorage& GetUserStorage();
  CharacterStorage& GetCharacterStorage();

private:
  std::unique_ptr<FileDataSource> _dataSource;

  DataStorage<std::string, data::User> _userStorage;
  DataStorage<std::string, data::Character> _characterStorage;
};

}

#endif //DATADIRECTOR_HPP
