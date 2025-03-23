//
// Created by rgnter on 25/11/2024.
//

#ifndef DATADIRECTOR_HPP
#define DATADIRECTOR_HPP

#include "Scheduler.hpp"
#include "server/Settings.hpp"

#include <cstdint>
#include <functional>
#include <future>
#include <mutex>
#include <unordered_map>

#include <pqxx/pqxx>

namespace alicia
{

//! Unique datum identifier.
using DatumUid = uint32_t;
//! Invalid datum identifier.
constexpr DatumUid InvalidDatumUid = 0;

namespace data
{

//! Item.
struct Item
{
  enum class Slot
  {
    Character,
    Horse,
    Storage,
  };

  //! A unique identifier.
  uint32_t uid{};
  //! A type identifier.
  uint32_t tid{};
  //! Amount of an item.
  uint32_t count{};

  Slot slot{Slot::Storage};
};

//! Character.
struct Character
{
  //!
  std::string name;
  //!
  std::string status;
  //!
  uint16_t level{};
  //!
  int32_t carrots{};
  //!
  int32_t cash{};

  //!
  enum class AgeGroup
  {
    Kid,
    Teenager,
    Highschooler,
    Adult
  } ageGroup = AgeGroup::Kid;

  //!
  enum class Gender
  {
    Unspecified,
    Boy,
    Girl
  } gender = Gender::Unspecified;

  //!
  struct Appearance
  {
    //!
    uint8_t charId{};
    //! FaceId
    uint8_t mouthSerialId{};
    //! EyeId
    uint8_t faceSerialId{};
    //! FigFace
    uint16_t headVolume{};
    //! FigTall
    uint16_t height{};
    //! FigVolume
    uint16_t thighVolume{};
    //! FigShape
    uint16_t legVolume{};
  } appearance{};

  //!
  std::vector<DatumUid> characterEquipment;
  //!
  std::vector<DatumUid> horseEquipment;

  //!
  DatumUid mountUid{};
  //!
  DatumUid ranchUid{};

  //!
  std::vector<DatumUid> horses{};
};

//! Horsie.
struct Horse
{
  //!
  uint32_t uid{};
  //!
  uint32_t tid{};
  //! Max length is 255.
  std::string name{};

  //!
  struct Appearance
  {
    //!
    uint8_t skinId{};
    //!
    uint8_t maneId{};
    //!
    uint8_t tailId{};
    //!
    uint8_t faceId{};
    //!
    uint8_t scale{};
    //!
    uint8_t legLength{};
    //!
    uint8_t legVolume{};
    //!
    uint8_t bodyLength{};
    //!
    uint8_t bodyVolume{};
  } appearance{};

  //!
  struct Stats
  {
    //!
    uint32_t agility{};
    //!
    uint32_t spirit{};
    //!
    uint32_t speed{};
    //!
    uint32_t strength{};
    //!
    uint32_t ambition{};
  } stats{};

  //!
  uint32_t rating{};
  //!
  uint8_t clazz{};
  //!
  uint8_t grade{};
  //!
  uint16_t growthPoints{};

  struct Mastery
  {
    uint32_t magic{};
    uint32_t jumping{};
    uint32_t sliding{};
    //! Divided by 10?
    uint32_t gliding{};
  } mastery{};
};

//! Ranch
struct Ranch
{
  std::string name;
};

//! User
struct User
{
  DatumUid characterUid;
  std::string name;
  std::string token;
};

} // namespace data

//! Data record with access mutex.
template<typename T>
struct Record
{
  //! Whether the record is available.
  std::atomic_bool available{false};
  //! An access mutex.
  std::mutex accessMutex{};
  //! Value.
  T value{};

  void Mutation(const std::function<void(T&)> mutation)
  {
    std::scoped_lock recordLock(accessMutex);
    mutation(value);
  }

  //! Data record view.
  struct View
  {
    //! Constructor for non-empty view.
    //! @param record The associated record.
    explicit View(Record* record)
        : _record(record)
        , _lock(_record->accessMutex, std::defer_lock)
    {
    }

    View()
      : _record(nullptr)
    {
    }

    //! Deleted copy constructor.
    View(const View&) = delete;
    //! Deleted copy assignment operator.
    void operator=(const View&) = delete;

    //! Move constructor.
    View(View&& other) noexcept
        : _record(other._record)
        , _lock(std::move(other._lock))
    {
    }
    //! Move assignment operator.
    View& operator=(View&& other) noexcept
    {
      _record = other._record;
      _lock = std::move(other._lock);
      return *this;
    }

    //! Returns whether the value is available.
    //! @returns `true` if the value is available, otherwise returns `false`.
    [[nodiscard]] bool IsAvailable() const
    {
      return _record && _record->available;
    }

    //! Returns the reference to the value.
    //! @returns Value.
    T& operator()()
    {
      assert(_record != nullptr && "Accessing null record");
      _lock.lock();
      return _record->value;
    }

  private:
    Record* _record;
    std::unique_lock<std::mutex> _lock{};
  };

  View GetView()
  {
    return View(this);
  }
};

class DataDirector
{
public:
  //!
  explicit DataDirector(
    Settings::DataSource settings = {});

  //! Establishes connection with the data source.
  void EstablishConnection();

  void PreloadUser(const std::string& name);
  //! Get the user record.
  //! @param name Name of the use.
  //! @returns View of the user record.
  Record<data::User>::View GetUser(std::string const &name);

  void PreloadCharacter(DatumUid characterUid);
  //! Get the character record.
  //! @param characterUid Uid of the character.
  //! @returns View of the character record.
  Record<data::Character>::View GetCharacter(DatumUid characterUid);

  void PreloadItems(const std::span<DatumUid>& itemUids);
  //! Get the item records.
  //! @param itemUids Uids of the items.
  //! @returns Views of the item records.
  std::vector<Record<data::Item>::View> GetItems(
    const std::span<DatumUid>& itemUids);

  void PreloadHorse(DatumUid horseUid);
  //! Get the horse record.
  //! @param horseUid Uid of the horse.
  //! @returns View of the horse record.
  Record<data::Horse>::View GetHorse(DatumUid horseUid);

  void PreloadRanch(DatumUid ranchUid);
  //! Get the ranch record.
  //! @param ranchUid Uid of the ranch.
  //! @returns View of the character record.
  Record<data::Ranch>::View GetRanch(DatumUid ranchUid);

  void PreloadHorses(const std::span<DatumUid>& horseUids);
  //! Get the horse records.
  //! @param horseUids Uids of the horses.
  //! @returns Views of the horse records.
  std::vector<Record<data::Horse>::View> GetHorses(
    const std::span<DatumUid>& horseUids);

private:
  //!
  std::unordered_map<
    std::string, Record<data::User>> _users;
  //!
  std::unordered_map<
    DatumUid, Record<data::Character>> _characters;
  //!
  std::unordered_map<
    DatumUid, Record<data::Item>> _items;
  //!
  std::unordered_map<
    DatumUid, Record<data::Ranch>> _ranches;
  //!
  std::unordered_map<
    DatumUid, Record<data::Horse>> _horses;

  //!
  Settings::DataSource _settings;
  //!
  server::TaskLoop _taskLoop;

  //!
  std::mutex _connectionMtx{};
  //!
  std::unique_ptr<pqxx::connection> _connection{nullptr};
};

}

#endif //DATADIRECTOR_HPP