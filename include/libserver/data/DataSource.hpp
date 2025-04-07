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
  virtual void RetrieveUser(const data::User&) = 0;
  virtual void StoreUser(const data::User&) = 0;
};


}

#endif //DATASOURCE_HPP
