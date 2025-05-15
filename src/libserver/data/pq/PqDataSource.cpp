//
// Created by maros on 4/7/25.
//

#include "libserver/data/pq/PqDataSource.hpp"

namespace soa
{

namespace
{

constexpr std::string_view UserTableName = "data.users";

} // namespace

void PqDataSource::Establish(const std::string& url)
{
  _connection = std::make_unique<pqxx::connection>(url);
}

bool PqDataSource::IsConnectionFine()
{
  return _connection and _connection->is_open();
};

void PqDataSource::RetrieveUser(data::User& user)
{
  try
  {
    pqxx::work query(*_connection);

    const auto result = query.exec_params1(
      "select (uid, token, characterUid) "
      "from $1 where name=$2",
      pqxx::params(UserTableName, user.name()));

    if (user.uid.IsModified())
      user.uid = result["uid"].as<data::Uid>(data::InvalidUid);
    if (user.token.IsModified())
      user.token = result["token"].as<std::string>({});
    if (user.characterUid.IsModified())
      user.characterUid = result["characterUid"].as<data::Uid>(data::InvalidUid);
  }
  catch (const std::exception& x)
  {
    throw x;
  }
}

void PqDataSource::StoreUser(const data::User& user)
{
  try
  {
    pqxx::work query(*_connection);

    const auto result = query.exec_params0(
      "insert into $1 (token, characterUid) VALUES ($2, $3)",
      pqxx::params(
        UserTableName,
        user.token(),
        user.characterUid()));
  }
  catch (const std::exception& x)
  {
    throw x;
  }
}

} // namespace soa
