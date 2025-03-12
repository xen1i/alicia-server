//
// Created by rgnter on 25/11/2024.
//

#include "server/DataDirector.hpp"

#include <spdlog/spdlog.h>

namespace
{

//!
const std::string QueryUserTokenRecordStatementId = "queryUserTokenRecord";
//!
const std::string QueryUserRecordStatementId = "queryUserRecord";
//!
const std::string QueryCharacterRecordStatementId = "queryCharacterRecord";
//!
const std::string QueryHorseRecordStatementId = "queryHorseRecord";
//!
const std::string QueryItemRecordStatementId = "queryItemRecord";
//!
const std::string QueryRanchRecordStatementId = "queryRanchRecord";

} // anon namespace


namespace alicia
{

DataDirector::DataDirector(Settings::DataSource settings)
    : _settings(std::move(settings))
{
  try
  {
    _connection = std::make_unique<pqxx::connection>(
      _settings.connectionString.c_str());

    _connection->prepare(
      QueryUserTokenRecordStatementId,
      "SELECT token, user_uid FROM data.token WHERE login=$1");
    _connection->prepare(
      QueryUserRecordStatementId,
      "SELECT * FROM data.user WHERE uid=$1");
    _connection->prepare(
      QueryCharacterRecordStatementId,
      "SELECT * FROM data.character WHERE uid=$1");
    _connection->prepare(
      QueryHorseRecordStatementId,
      "SELECT * FROM data.horse WHERE uid=$1");
    _connection->prepare(
      QueryRanchRecordStatementId,
      "SELECT * FROM data.ranch WHERE uid=$1");
    _connection->prepare(
      QueryItemRecordStatementId,
      "SELECT * FROM data.item WHERE uid=$1");

    spdlog::info(
      "Initialized the data source with connection string '{}'",
      _settings.connectionString);
  }
  catch (const std::exception& x)
  {
    spdlog::error(
      "Failed to initialize the data source with connection string '{}' because: {}",
      _settings.connectionString,
      x.what());
  }
}

void DataDirector::GetToken(
  const std::string& user,
  const std::function<void(View<data::Token>&&)>& consumer,
  const std::function<void()>& errorConsumer)
{
  if (not _connection)
  {
    errorConsumer();
    return;
  }

  try
  {
    pqxx::work query(*_connection);

    // Query and find the user login and the token.
    const auto result = query.exec_prepared1(
      QueryUserTokenRecordStatementId, user.c_str());

    data::Token token{};
    if (result.num())
    {
      token = {
        .userUid = result["user_uid"].as<uint32_t>(),
        .token = result["token"].as<std::string>()};
    }

    consumer(View(token));
  }
  catch (std::exception& x)
  {
    spdlog::error("DataDirector error: {}", x.what());
    errorConsumer();
  }
}

void DataDirector::GetUser(
  uint32_t userUid,
  const std::function<void(View<data::User>&&)>& consumer,
  const std::function<void()>& errorConsumer)
{
  // Return the user from the cache if possible.
  if (const auto userIter = _users.find(userUid);
      userIter != _users.cend())
  {
    consumer(View(userIter->second.value));
    return;
  }

  try
  {
    pqxx::work query(*_connection);

    // Query and find the user login and the token.
    const auto result = query.exec_prepared1(QueryUserRecordStatementId, userUid);

    auto& userRecord = _users[userUid];
    consumer(View(userRecord.value));
  }
  catch (std::exception& x)
  {
    spdlog::error("DataDirector error: {}", x.what());
    errorConsumer();
  }
}

} // namespace alicia
