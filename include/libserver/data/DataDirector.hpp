//
// Created by maros on 4/7/25.
//

#ifndef DATADIRECTOR_HPP
#define DATADIRECTOR_HPP

#include "DataDefinitions.hpp"
#include "DataStorage.hpp"
#include "pq/PqDataSource.hpp"

namespace soa
{

class DataDirector
{
public:
  //! Default constructor.
  //! @param url pq url
  explicit DataDirector(const std::string& url);
  //! Ticks the data director.
  void Tick();

  bool IsUserAvailable(const std::string& name);
  data::User& GetUser(const std::string& name);

private:
  PqDataSource _dataSource;
  DataStorage<std::string, data::User> _userStorage;
};

}

#endif //DATADIRECTOR_HPP
