//
// Created by maros on 4/6/25.
//

#ifndef DATADEFINITIONS_HPP
#define DATADEFINITIONS_HPP

#include <atomic>
#include <cstdint>
#include <functional>
#include <string>

namespace soa
{

namespace dao
{

template<typename T>
struct Field
{
  //! Constructs a field with an initialized value.
  //! @param value Value.
  Field(T value) noexcept
    : _value(std::move(value))
  {
  }

  //! Constructs field with an initialized value.
  Field()
    : _value()
  {
  }

  //! Constructs a field with an uninitialized value.
  explicit Field(nullptr_t t)
  {
  }

  bool IsModified() const noexcept
  {
    return _modified;
  }

  T& operator()(const T& value) noexcept
  {
    _modified = true;
    _value = value;
    return value;
  }

  T& operator()(T&& value) noexcept
  {
    _modified = true;
    _value = std::move(value);
    return value;
  }

  const T& operator()() const noexcept
  {
    return _value;
  }

  T& operator()() noexcept
  {
    return _value;
  }

private:
  std::atomic_bool _modified{false};
  T _value;
};

}

namespace data
{

//! Unique identifier.
using Uid = uint32_t;
//! Type identifier.
using Tid = uint32_t;
//! Value of an invalid unique identifier.
constexpr Uid InvalidUid = 0;
//! Value of an invalid type identifier.
constexpr Tid InvalidTid = 0;

//! User
struct User
{
  //! A name of the user.
  dao::Field<std::string> name{};
  //! An authorization token of the user.
  dao::Field<std::string> token{};
  //! A character UID of the user.
  dao::Field<Uid> characterUid{InvalidUid};
};

//! Item
struct Item
{
  //! Slot type of the item.
  enum class Slot
  {
    CharacterEquipment,
    HorseEquipment,
    Storage,
  };

  //! A unique identifier.
  dao::Field<Uid> uid{InvalidUid};
  //! A type identifier.
  dao::Field<Tid> tid{InvalidTid};
  //! Amount of an item.
  dao::Field<uint32_t> count{};
  //! Slot of the item.
  dao::Field<Slot> slot{Slot::Storage};
};

} // namespace data

} // namespace soa

#endif //DATADEFINITIONS_HPP
