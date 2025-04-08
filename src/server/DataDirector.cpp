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
const std::string QueryCharacterEquipmentListStatementId = "queryUserCharacterItems";
//!
const std::string QueryCharacterHorseEquipmentListStatementId = "queryUserHorseItems";
//!
const std::string QueryCharacterHorseListStatementId = "queryUserHorses";
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
//! @param characterResult Result to compose from.
//! @param characterEquipmentResult Result of character equipment to compose from.
//! @param horseEquipmentResult Result of horse equipment to compose from.
//! @param horsesResult Result of horses to compose from.
void ComposeCharacterFromResult(
  alicia::Record<alicia::data::Character>& record,
  const pqxx::row& characterResult,
  const pqxx::result& characterEquipmentResult,
  const pqxx::result& horseEquipmentResult,
  const pqxx::result& horsesResult)
{
  record.Mutation([&](auto& character)
    {
      character =  alicia::data::Character{
        .name = characterResult["name"].as<std::string>({}),
        .status = characterResult["status"].as<std::string>({}),
        .level = characterResult["level"].as<uint16_t>(0),
        .carrots = characterResult["carrots"].as<int32_t>(0),
        .cash = characterResult["cash"].as<int32_t>(0),
        .ageGroup = static_cast<alicia::data::Character::AgeGroup>(characterResult["ageGroup"].as<uint32_t>(
            static_cast<uint32_t>(alicia::data::Character::AgeGroup::Kid))),
        .gender = static_cast<alicia::data::Character::Gender>(characterResult["gender"].as<uint32_t>(
            static_cast<uint32_t>(alicia::data::Character::Gender::Unspecified))),
        .appearance = {
          .charId = characterResult["charId"].as<uint32_t>(0),
          .mouthSerialId = characterResult["mouthSerialId"].as<uint32_t>(0),
          .faceSerialId = characterResult["faceSerialId"].as<uint32_t>(0),
          .headVolume = characterResult["headVolume"].as<uint16_t>(0),
          .height = characterResult["height"].as<uint16_t>(0),
          .thighVolume = characterResult["thighVolume"].as<uint16_t>(0),
          .legVolume = characterResult["legVolume"].as<uint16_t>(0),},
      .mountUid = characterResult["mountUid"].as<alicia::DatumUid>({0}),
      .ranchUid = characterResult["ranchUid"].as<alicia::DatumUid>({0}),};

      for (auto characterEquipment : characterEquipmentResult)
      {
        character.characterEquipment.emplace_back(
          characterEquipment["uid"].as<alicia::DatumUid>());
      }

      for (auto horseEquipment : horseEquipmentResult)
      {
        character.horseEquipment.emplace_back(
          horseEquipment["uid"].as<alicia::DatumUid>());
      }

      for (auto horse : horsesResult)
      {
        character.horses.emplace_back(
          horse["uid"].as<alicia::DatumUid>());
      }
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
        .skinId = result["skinId"].as<uint32_t>(0),
        .maneId = result["maneId"].as<uint32_t>(0),
        .tailId = result["tailId"].as<uint32_t>(0),
        .faceId = result["faceId"].as<uint32_t>(0),
        .scale = result["scale"].as<uint32_t>(0),
        .legLength = result["legLength"].as<uint32_t>(0),
        .legVolume = result["legVolume"].as<uint32_t>(0),
        .bodyLength = result["bodyLength"].as<uint32_t>(0),
        .bodyVolume = result["bodyVolume"].as<uint32_t>(0),},
      .stats = {
        .agility = result["agility"].as<uint32_t>(0),
        .spirit = result["spirit"].as<uint32_t>(0),
        .speed = result["speed"].as<uint32_t>(0),
        .strength = result["strength"].as<uint32_t>(0),
        .ambition = result["ambition"].as<uint32_t>(0),},
      .rating = result["rating"].as<uint32_t>(0),
      .clazz = result["class"].as<uint32_t>(0),
      .grade = result["grade"].as<uint32_t>(0),
      .growthPoints = result["growthPoints"].as<uint16_t>(0),
      .mastery = {
        .magic = result["magic"].as<uint32_t>(0),
        .jumping = result["jumping"].as<uint32_t>(0),
        .sliding = result["sliding"].as<uint32_t>(0),
        .gliding = result["gliding"].as<uint32_t>(0),},};
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
      QueryUserRecordStatementId,
      "SELECT * FROM data.user "
      "WHERE name=$1");

    // Character record query.
    _connection->prepare(
      QueryCharacterRecordStatementId,
      "SELECT * FROM data.character "
      "WHERE uid=$1");

    // Character equipment items query.
    _connection->prepare(
      QueryCharacterEquipmentListStatementId,
      "SELECT uid FROM data.item "
      "WHERE character_uid=$1 AND slot=CharacterEquipment");

    // Character horse equipment query.
    _connection->prepare(
      QueryCharacterHorseEquipmentListStatementId,
      "SELECT uid FROM data.item "
      "WHERE character_uid=$1 AND slot=HorseEquipment");

    // Character horse list query.
    _connection->prepare(
      QueryCharacterHorseListStatementId,
      "SELECT uid FROM data.horse "
      "WHERE character_uid=$1");

    // Horse record query.
    _connection->prepare(QueryHorseRecordStatementId,
      "SELECT * FROM data.horse "
      "WHERE uid=$1");

    // Ranch record query.
    _connection->prepare(QueryRanchRecordStatementId,
      "SELECT * FROM data.ranch "
      "WHERE uid=$1");

    // Item record query.
    _connection->prepare(QueryItemRecordStatementId,
      "SELECT * FROM data.item "
      "WHERE uid=$1");

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

        const auto characterResult = query.exec_prepared1(
          QueryCharacterRecordStatementId, characterUid);

        const auto characterEquipmentList = query.exec_prepared(
          QueryCharacterEquipmentListStatementId, characterUid);
        const auto horseEquipmentList = query.exec_prepared(
          QueryCharacterHorseEquipmentListStatementId, characterUid);

        const auto horseList = query.exec_prepared(
          QueryCharacterHorseListStatementId, characterUid);

        assert(_characters.contains(characterUid) && "Record must exist");
        auto& record = _characters[characterUid];
        ComposeCharacterFromResult(
          record,
          characterResult,
          characterEquipmentList,
          horseEquipmentList,
          horseList);
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

void DataDirector::PreloadItems(
  const std::span<DatumUid>& itemUids)
{
  _taskLoop.Queue(
    [this, itemUids]()
    {
      try
      {
        pqxx::work query(*_connection);

        const std::string itemList = [&itemUids]()
        {
          std::string list;
          for (auto it = itemUids.begin(); it != itemUids.end();)
          {
            list += fmt::format("{}", *it);
            ++it;

            if (it != itemUids.end())
              list += ", ";
          }

          return list;
        }();

        // Query for the character.
        const auto result = query.exec(std::format(
          "SELECT * FROM data.item "
          "WHERE uid in ({})",
          itemList));

        for (const auto& itemResult : result)
        {
          const auto itemUid = itemResult["uid"].as<DatumUid>(
            InvalidDatumUid);
          assert(itemUid != InvalidDatumUid && "Data invalid");

          assert(_items.contains(itemUid) && "Record must exist");
          ComposeItemFromResult(_items[itemUid], itemResult);
        }
      }
      catch (const std::exception& x)
      {
        spdlog::error("DataDirector error: {}", x.what());
      }
    });
}

std::vector<Record<data::Item>::View> DataDirector::GetItems(
  const std::span<DatumUid>& itemUids)
{
  std::vector<Record<data::Item>::View> items;
  std::vector<DatumUid> itemsToPreload;

  for (const auto& itemUid : itemUids)
  {
    const auto [iterator, inserted] = _items.try_emplace(itemUid);

    // The item was just inserted, queue the preload.
    if (inserted)
      itemsToPreload.emplace_back(itemUid);

    items.emplace_back() = std::move(iterator->second.GetView());
  }

  if (not itemsToPreload.empty())
    PreloadItems(std::span(itemsToPreload.begin(), itemsToPreload.end()));

  return items;
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

void DataDirector::PreloadHorses(
  const std::span<DatumUid>& horseUids)
{
  _taskLoop.Queue(
    [this, horseUids]()
    {
      try
      {
        pqxx::work query(*_connection);

        const std::string horseList = [&horseUids]()
        {
          std::string list;
          for (auto it = horseUids.begin(); it != horseUids.end();)
          {
            list += fmt::format("{}", *it);
            ++it;

            if (it != horseUids.end())
              list += ", ";
          }

          return list;
        }();

        // Query for the horses.
        const auto result = query.exec(std::format(
          "SELECT * FROM data.horse "
          "WHERE uid in ({})",
          horseList));

        for (const auto& horseResult : result)
        {
          const auto horseUid = horseResult["uid"].as<DatumUid>(
            InvalidDatumUid);
          assert(horseUid != InvalidDatumUid && "Data invalid");

          assert(_horses.contains(horseUid) && "Record must exist");
          ComposeHorseFromResult(_horses[horseUid], horseResult);
        }
      }
      catch (const std::exception& x)
      {
        spdlog::error("DataDirector error: {}", x.what());
      }
    });
}

std::vector<Record<data::Horse>::View> DataDirector::GetHorses(
  const std::span<DatumUid>& horseUids)
{
  std::vector<Record<data::Horse>::View> horses;
  std::vector<DatumUid> horsesToPreload;

  for (const auto& horseUid : horseUids)
  {
    const auto [iterator, inserted] = _horses.try_emplace(horseUid);

    // The horse was just inserted, queue the preload.
    if (inserted)
      horsesToPreload.emplace_back(horseUid);

    horses.emplace_back() = std::move(iterator->second.GetView());
  }

  if (not horsesToPreload.empty())
    PreloadHorses(std::span(horsesToPreload.begin(), horsesToPreload.end()));

  return horses;
}

} // namespace alicia
