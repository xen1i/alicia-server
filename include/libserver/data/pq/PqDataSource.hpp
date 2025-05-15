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
  //! Establishes the connection to the data source.
  void Establish(const std::string& url);
  //! Returns whether the connection is fine.
  //! @returns `true` if the connection is fine, `false` otherwise.
  bool IsConnectionFine();

  void RetrieveUser(data::User&) override;
  void StoreUser(const data::User&) override;

private:
  std::unique_ptr<pqxx::connection> _connection;
};

}

#endif //PQDATASOURCE_HPP
