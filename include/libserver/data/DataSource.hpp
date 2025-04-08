//
// Created by maros on 4/6/25.
//

#ifndef DATASOURCE_HPP
#define DATASOURCE_HPP

#include "server/Settings.hpp"
#include "libserver/data/DataDefinitions.hpp"

namespace soa
{

//! A class managing a data source.
class DataSource
{
public:
  //! Default destructor.
  virtual ~DataSource() = default;
  //! Retrieves the user from the data source.
  //! @param user User to retrieve.
  virtual void RetrieveUser(data::User& user) = 0;
  //! Stores the user on the data source.
  //! @param user User to store.
  virtual void StoreUser(const data::User& user) = 0;
};


}

#endif //DATASOURCE_HPP
