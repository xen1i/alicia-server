//
// Created by maros on 4/7/25.
//

#ifndef PQDATASOURCE_HPP
#define PQDATASOURCE_HPP

#include "SqlUtils.hpp"

#include <libserver/data/DataDefinitions.hpp>
#include <libserver/data/DataSource.hpp>
#include <pqxx/pqxx>

namespace soa
{

namespace
{

//! Composes user from result.
//! @param record User record.
//! @param result Result to compose from.
void ComposeUserFromResult(
  data::User& user,
  const pqxx::row& result)
{
  // user = data::User{
  //   .name{result["name"].as<std::string>({})},
  //   .token = result["token"].as<std::string>({}),
  //   .characterUid{result["characterUid"].as(data::InvalidUid)},};
}

std::string DecomposeToQuery(
  const data::User& user)
{
  SqlComposer composer;
  composer.Parameter(
    "name",
    [&]{ return user.name.IsModified(); },
    [&] { return user.name(); });
  composer.Parameter(
    "token",
    [&]{ return false; },
    [&] { return user.token(); });
  composer.Parameter(
    "characterUid",
    [&]{ return false; },
    [&] { return std::format("{}", user.characterUid()); });

  return composer.BuildUpdate("users");
}

}

class PqDataSource
  : public DataSource
{
  data::User GetUser()
  {
    data::User user;
    return {};
  }

private:

};

}

#endif //PQDATASOURCE_HPP
