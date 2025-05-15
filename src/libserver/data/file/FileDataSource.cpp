#include "libserver/data/file/FileDataSource.hpp"

#include <fstream>

#include <nlohmann/json.hpp>

namespace
{

std::filesystem::path ProduceDataPath(
  const std::filesystem::path& root,
  const std::string& filename)
{
  if (not std::filesystem::exists(root))
    std::filesystem::create_directories(root);
  return root / filename / ".json";
}

}

void soa::FileDataSource::Initialize(const std::filesystem::path& path)
{
  _path = path;
  _usersPath = _path / "users";
  create_directories(_usersPath);
  _charactersPath = _path / "characters";
  create_directories(_charactersPath);
  _metaFilePath = _path/ "meta.json";

  const std::filesystem::path metaFilePath = ProduceDataPath(
    _metaFilePath, "meta");
  std::ifstream metaFile(metaFilePath);
  if (not metaFile.is_open())
  {
    return;
  }

  const auto meta = nlohmann::json::parse(metaFile);
  _sequentialUserId = meta["sequentialUserId"].get<uint32_t>();
  _sequentialCharacterId = meta["sequentialCharacterId"].get<uint32_t>();
}

void soa::FileDataSource::Terminate()
{
  const std::filesystem::path metaFilePath = ProduceDataPath(
    _metaFilePath, "meta");

  std::ofstream metaFile(metaFilePath);
  if (not metaFile.is_open())
  {
    return;
  }

  nlohmann::json meta;
  meta["sequentialUserId"] = _sequentialUserId;
  meta["sequentialCharacterId"] = _sequentialCharacterId;

  metaFile << meta.dump(2);
}

void soa::FileDataSource::RetrieveUser(data::User& user)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _usersPath, user.name());

  std::ifstream file(dataFilePath);
  if (not file.is_open())
    return;

  const auto json = nlohmann::json::parse(file);
  user.token = json["token"].get<std::string>();
  user.characterUid = json["characterUid"].get<data::Uid>();
}

void soa::FileDataSource::StoreUser(const data::User& user)
{
  const std::filesystem::path userFilePath = ProduceDataPath(
    _usersPath, user.name());

  std::ofstream file(userFilePath);
  if (not file.is_open())
    return;

  nlohmann::json json;
  json["token"] = user.token();
  json["characterUid"] = user.characterUid();

  file << json.dump(2);
}

void soa::FileDataSource::RetrieveItem(data::Item& item)
{

}

void soa::FileDataSource::StoreItem(const data::Item& item)
{

}

void soa::FileDataSource::RetrieveCharacter(data::Character& character)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _charactersPath, std::format("{}", character.uid()));

  std::ifstream file(dataFilePath);
  if (not file.is_open())
    return;

  const auto json = nlohmann::json::parse(file);

  character.name = json["name"].get<std::string>();
  character.parts = data::Character::Parts{
    .modelId = json["modelId"].get<data::Uid>(),
    .mouthId = json["mouthId"].get<data::Uid>(),
    .faceId = json["faceId"].get<data::Uid>()};
  character.appearance = data::Character::Appearance{
    .headSize = json["headSize"].get<uint32_t>(),
    .height = json["height"].get<uint32_t>(),
    .thighVolume = json["thighVolume"].get<uint32_t>(),
    .legVolume = json["legVolume"].get<uint32_t>()};
}

void soa::FileDataSource::StoreCharacter(const data::Character& character)
{
  const std::filesystem::path userFilePath = ProduceDataPath(
    _charactersPath, std::format("{}", character.uid()));

  std::ofstream file(userFilePath);
  if (not file.is_open())
    return;

  nlohmann::json json;
  json["name"] = character.name();

  // character parts
  nlohmann::json parts;
  parts["modelId"] = character.parts.modelId();
  parts["mouthId"] = character.parts.mouthId();
  parts["faceId"] = character.parts.faceId();
  json["parts"] = parts;

  // character appearance
  nlohmann::json appearance;
  parts["headSize"] = character.appearance.headSize();
  parts["height"] = character.appearance.height();
  parts["thighVolume"] = character.appearance.thighVolume();
  parts["legVolume"] = character.appearance.legVolume();
  json["appearance"] = appearance;

  file << json.dump(2);
}
