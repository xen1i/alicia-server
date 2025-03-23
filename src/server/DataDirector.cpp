#include "server/DataDirector.hpp"

#include "libserver/command/proto/DataDefines.hpp"

#include <spdlog/spdlog.h>

namespace
{

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

//! Composes user from result.
//! @param record User record.
//! @param result Result to compose from.
void ComposeUserFromResult(
  alicia::Record<alicia::data::User>& record,
  const pqxx::row& result)
{
  record.Mutation([&result](auto& user)
    {
      user = alicia::data::User{
        .characterUid = result["characterUid"].as(alicia::InvalidDatumUid),
        .name = result["name"].as<std::string>({}),
        .token = result["token"].as<std::string>({}),};
    });
}

//! Composes character from result.
//! @param record Character record.
//! @param result Result to compose from.
void ComposeCharacterFromResult(
  alicia::Record<alicia::data::Character>& record,
  const pqxx::row& result)
{
  record.Mutation([&result](auto& character)
    {
      character =  alicia::data::Character{
        .name = result["name"].as<std::string>({}),
        .status = result["status"].as<std::string>({}),
        .level = result["level"].as<uint16_t>(0),
        .carrots = result["carrots"].as<int32_t>(0),
        .cash = result["cash"].as<int32_t>(0),
        .ageGroup = static_cast<alicia::data::Character::AgeGroup>(result["ageGroup"].as<uint32_t>(
            static_cast<uint32_t>(alicia::data::Character::AgeGroup::Kid))),
        .gender = static_cast<alicia::data::Character::Gender>(result["gender"].as<uint32_t>(
            static_cast<uint32_t>(alicia::data::Character::Gender::Unspecified))),
        .appearance = {
          .charId = result["charId"].as<uint8_t>(0),
          .mouthSerialId = result["mouthSerialId"].as<uint8_t>(0),
          .faceSerialId = result["faceSerialId"].as<uint8_t>(0),
          .headVolume = result["headVolume"].as<uint16_t>(0),
          .height = result["height"].as<uint16_t>(0),
          .thighVolume = result["thighVolume"].as<uint16_t>(0),
          .legVolume = result["legVolume"].as<uint16_t>(0),},
      .mountUid = result["mountUid"].as<alicia::DatumUid>({0}),
      .ranchUid = result["ranchUid"].as<alicia::DatumUid>({0}),};
    });
}

//! Composes horse from result.
//! @param record Horse record.
//! @param result Result to compose from.
void ComposeHorseFromResult(
  alicia::Record<alicia::data::Horse>& record,
  const pqxx::row& result)
{
  record.Mutation([&result](auto& horse)
  {
    horse = alicia::data::Horse{
      .uid = result["uid"].as<uint32_t>(0),
      .tid = result["tid"].as<uint32_t>(0),
      .name = result["name"].as<std::string>({}),
      .appearance = {
        .skinId = result["skinId"].as<uint8_t>(0),
        .maneId = result["maneId"].as<uint8_t>(0),
        .tailId = result["tailId"].as<uint8_t>(0),
        .faceId = result["faceId"].as<uint8_t>(0),
        .scale = result["scale"].as<uint8_t>(0),
        .legLength = result["legLength"].as<uint8_t>(0),
        .legVolume = result["legVolume"].as<uint8_t>(0),
        .bodyLength = result["bodyLength"].as<uint8_t>(0),
        .bodyVolume = result["bodyVolume"].as<uint8_t>(0),},
      .stats = {
        .agility = result["agility"].as<uint8_t>(0),
        .spirit = result["spirit"].as<uint8_t>(0),
        .speed = result["speed"].as<uint8_t>(0),
        .strength = result["strength"].as<uint8_t>(0),
        .ambition = result["ambition"].as<uint8_t>(0),},
      .rating = result["rating"].as<uint32_t>(0),
      .clazz = result["class"].as<uint8_t>(0),
      .grade = result["grade"].as<uint8_t>(0),
      .growthPoints = result["growthPoints"].as<uint16_t>(0),
      .mastery = {
        .magic = result["magic"].as<uint8_t>(0),
        .jumping = result["jumping"].as<uint8_t>(0),
        .sliding = result["sliding"].as<uint8_t>(0),
        .gliding = result["gliding"].as<uint8_t>(0),},};
  });
}

//! Composes ranch from result.
//! @param record ranch record.
//! @param result Result to compose from.
void ComposeRanchFromResult(
  alicia::Record<alicia::data::Ranch>& record,
  const pqxx::row& result)
{
  record.Mutation([&result](auto& ranch)
  {
    ranch = alicia::data::Ranch{
      .name = result["name"].as<std::string>({}),};
  });
}

//! Composes ranch from result.
//! @param record ranch record.
//! @param result Result to compose from.
void ComposeItemFromResult(
  alicia::Record<alicia::data::Item>& record,
  const pqxx::row& result)
{
  record.Mutation([&result](auto& item)
  {
    item = alicia::data::Item{
      .uid = result["uid"].as<uint32_t>(alicia::InvalidDatumUid),
      .tid = result["tid"].as<uint32_t>(alicia::InvalidDatumUid),
      .count = result["count"].as<uint32_t>(alicia::InvalidDatumUid),};
  });
}

} // anon namespace


namespace alicia
{

DataDirector::DataDirector(Settings::DataSource settings)
  : _settings(std::move(settings))
{
  _taskLoop.Begin();
}

void DataDirector::EstablishConnection()
{
  try
  {
    _connection = std::make_unique<pqxx::connection>(_settings.connectionString.c_str());

    // User record query.
    _connection->prepare(
      QueryUserRecordStatementId, "SELECT * FROM data.user WHERE name=$1");
    // Character record query.
    _connection->prepare(
      QueryCharacterRecordStatementId, "SELECT * FROM data.character WHERE uid=$1");
    // Horse record query.
    _connection->prepare(QueryHorseRecordStatementId, "SELECT * FROM data.horse WHERE uid=$1");
    // Ranch record query.
    _connection->prepare(QueryRanchRecordStatementId, "SELECT * FROM data.ranch WHERE uid=$1");
    // Item record query.
    _connection->prepare(QueryItemRecordStatementId, "SELECT * FROM data.item WHERE uid=$1");

    spdlog::info(
      "Initialized the data source with the connection string '{}'", _settings.connectionString);
  }
  catch (const std::exception& x)
  {
    spdlog::error(
      "Failed to establish the data source connection with connection string '{}' because: {}",
      _settings.connectionString,
      x.what());
  }
}

void DataDirector::PreloadUser(const std::string& name)
{
  _taskLoop.Queue(
    [this, name]()
    {
      try
      {
        pqxx::work query(*_connection);

        // Query for the user.
        const auto result = query.exec_prepared1(
         QueryUserRecordStatementId, name);

        assert(_users.contains(name) && "Record must exist");
        auto& record = _users[name];
        ComposeUserFromResult(record, result);
        record.available = true;
      }
      catch (const std::exception& x)
      {
       spdlog::error("DataDirector error: {}", x.what());
      }
    });
}

Record<data::User>::View DataDirector::GetUser(std::string const& name)
{
  const auto [iterator, inserted] = _users.try_emplace(name);

  // The value was just inserted, queue the preload.
  if (inserted)
    PreloadUser(name);

  return std::move(iterator->second.GetView());
}

void DataDirector::PreloadCharacter(DatumUid characterUid)
{
  _taskLoop.Queue(
    [this, characterUid]()
    {
      try
      {
        pqxx::work query(*_connection);

        // Query for the character.
        const auto result = query.exec_prepared1(QueryUserRecordStatementId, characterUid);

        assert(_characters.contains(characterUid) && "Record must exist");
        auto& record = _characters[characterUid];
        ComposeCharacterFromResult(record, result);
      }
      catch (const std::exception& x)
      {
        spdlog::error("DataDirector error: {}", x.what());
      }
    });
}

Record<data::Character>::View DataDirector::GetCharacter(DatumUid characterUid)
{
  const auto [iterator, inserted] = _characters.try_emplace(characterUid);

  // The value was just inserted, queue the preload.
  if (inserted)
    PreloadCharacter(characterUid);

  return std::move(iterator->second.GetView());
}

void DataDirector::PreloadHorse(
  const DatumUid horseUid)
{
  _taskLoop.Queue(
    [this, horseUid]()
    {
      try
      {
        pqxx::work query(*_connection);

        // Query for the character.
        const auto result = query.exec_prepared1(QueryHorseRecordStatementId, horseUid);

        assert(_horses.contains(horseUid) && "Record must exist");
        auto& record = _horses[horseUid];
        ComposeHorseFromResult(record, result);
        record.available = true;
      }
      catch (const std::exception& x)
      {
        spdlog::error("DataDirector error: {}", x.what());
      }
    });
}

Record<data::Horse>::View DataDirector::GetHorse(
  const DatumUid horseUid)
{
  const auto [iterator, inserted] = _horses.try_emplace(horseUid);

  // The value was just inserted, queue the preload.
  if (inserted)
    PreloadHorse(horseUid);

  return std::move(iterator->second.GetView());
}

void DataDirector::PreloadRanch(DatumUid ranchUid)
{
  _taskLoop.Queue(
    [this, ranchUid]()
    {
      try
      {
        pqxx::work query(*_connection);

        // Query for the character.
        const auto result = query.exec_prepared1(QueryRanchRecordStatementId, ranchUid);

        assert(_ranches.contains(ranchUid) && "Record must exist");
        auto& record = _ranches[ranchUid];
        ComposeRanchFromResult(record, result);
        record.available = true;
      }
      catch (const std::exception& x)
      {
        spdlog::error("DataDirector error: {}", x.what());
      }
    });
}

Record<data::Ranch>::View DataDirector::GetRanch(const DatumUid ranchUid)
{
  const auto [iterator, inserted] = _ranches.try_emplace(ranchUid);

  // The value was just inserted, queue the preload.
  if (inserted)
    PreloadRanch(ranchUid);

  return std::move(iterator->second.GetView());
}

void DataDirector::PreloadHorses() {}
Record<data::Horse>::View DataDirector::GetHorses(DatumUid characterUid)
{
  return {};
}

} // namespace alicia
