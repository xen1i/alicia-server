//
// Created by maros on 4/6/25.
//

#ifndef DATASTORAGE_HPP
#define DATASTORAGE_HPP

#include <atomic>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <spdlog/spdlog.h>

namespace soa
{

template<typename Key, typename Data>
class DataStorage
{
public:
  void CreateRecord(const Key& key, std::function<Data()> supplier)
  {
    auto& record = _records[key];
    record.value = supplier();
  }

  void UpdateRecord(const Key& key, std::function<void(Data&)> supplier)
  {
    auto& record = _records[key];
    supplier(record.value);
  }

  void DeleteRecord(const Key& key)
  {
    _records.erase(key);
  }

  Data* Get(const Key& key)
  {
    auto [recordIter, created] = _records.try_emplace(key);
    auto& record = recordIter->second;

    if (not record.synchronized)
      return nullptr;
    return &record.value;
  }

  void Tick()
  {
    for (const auto& recordKey : _recordsToStore)
    {
      auto& record = _records[recordKey];
      if (_recordStoreListener(recordKey, record))
      {
        record.synchronized = true;
        record.storePending = false;
      }
    }
    _recordsToStore.clear();

    for (const auto& recordKey : _recordsToRetrieve)
    {
      Record& record = _records[recordKey];
      if (_recordRetrieveListener(recordKey, record))
      {
        record.synchronized = true;
        record.retrievePending = false;
      }
    }
    _recordsToRetrieve.clear();
  }

private:
  struct Record
  {
    //! A flag indicating whether the record is modified.
    std::atomic_bool modified{false};
    //! A flag indicating whether the record is synchronized.
    std::atomic_bool synchronized{false};
    //! An access mutex.
    std::mutex mutex;

    Data value;
  };

  std::unordered_map<Key, Record> _records{};
  std::unordered_set<Key> _recordsToRetrieve{};
  std::unordered_set<Key> _recordsToStore{};
};

} // namespace soa

#endif //DATASTORAGE_HPP
