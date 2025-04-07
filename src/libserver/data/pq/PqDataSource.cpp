//
// Created by maros on 4/7/25.
//

#include "libserver/data/pq/PqDataSource.hpp"
#include "libserver/data/pq/SqlUtils.hpp"

namespace soa
{

namespace
{

const SqlDecomposer<data::User> userSqlDecomposer = []
{
  SqlDecomposer<data::User> decomposer;
  decomposer
    .Field(
      "uid",
      [](const auto& user, const auto& result)
      {
        user.uid = result.as<data::Uid>(data::InvalidUid);
      })
    .Field(
      "name",
      [](const auto& user, const auto& result)
      {
        user.name = result.as<std::string>({});
      })
    .Field(
      "token",
      [](const auto& user, const auto& result)
      {
        user.token = result.as<std::string>({});
      })
    .Field(
      "characterUid",
      [](const auto& user, const auto& result)
      {
        user.characterUid = result.as<data::Uid>(data::InvalidUid);
      });

  return decomposer;
}();

const SqlComposer<data::User> userSqlComposer = []()
{
  SqlComposer<data::User> composer;
  composer
    .Parameter(
      "uid",
      [&](const auto& user) { return user.uid.IsModified(); },
      [&](const auto& user) { return std::format("{}", user.uid()); })
    .Parameter(
      "token",
      [&](const auto& user) { return false; },
      [&](const auto& user) { return user.token(); })
    .Parameter(
      "characterUid",
      [&](const auto& user) { return false; },
      [&](const auto& user) { return std::format("{}", user.characterUid()); })
    .Condition(
      "name",
      [&](const auto& user) { return true; },
      [&](const auto& user) { return user.name(); });

  return composer;
}();

} // namespace

void PqDataSource::RetrieveUser(data::User& user)
{
  try
  {
    pqxx::work query(_connection);

    const auto result = query.exec_params1(
      "SELECT (username, token, characterUid) "
      "FROM data.users "
      "WHERE username=$1",
      user.name());

    userSqlDecomposer.Decompose(result, user);
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
    pqxx::work query(_connection);

    const auto result = query.exec_params0(
      userSqlComposer.ComposeUpsert();

  }
  catch (const std::exception& x)
  {
    throw x;
  }
}

} // namespace soa
