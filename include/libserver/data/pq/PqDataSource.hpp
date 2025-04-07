//
// Created by maros on 4/7/25.
//

#ifndef PQDATASOURCE_HPP
#define PQDATASOURCE_HPP

#include <libserver/data/DataDefinitions.hpp>
#include <libserver/data/DataSource.hpp>
#include <pqxx/pqxx>

namespace soa
{
class PqDataSource
  : public DataSource
{
public:
  void RetrieveUser(const data::User&) override;
  void StoreUser(const data::User&) override;

private:
  pqxx::connection _connection;
};

}

#endif //PQDATASOURCE_HPP
