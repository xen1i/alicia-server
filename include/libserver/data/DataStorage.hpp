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
  using DataSourceRetrieveListener = std::function<void(Data& data)>;
  using DataSourceStoreListener = std::function<void(Data& data)>;

  DataStorage(
    const DataSourceRetrieveListener& data_source_retrieve_listener,
    const DataSourceStoreListener& data_source_store_listener)
      : _dataSourceRetrieveListener(data_source_retrieve_listener)
      , _dataSourceStoreListener(data_source_store_listener)
  {
  }

  bool IsAvailable(const Key& key)
  {
    return _records[key].available;
  }

  Data& Get(const Key& key)
  {
    auto [recordIter, created] = _records.try_emplace(key);
    auto& record = recordIter->second;

    if (created)
      _retrieveQueue.insert(key);

    return record.value;
  }

  void Tick()
  {
    for (const auto& key : _retrieveQueue)
    {
      auto& userRecord = _records[key];
      _dataSourceRetrieveListener(userRecord.value);
      userRecord.available = true;
    }
    _retrieveQueue.clear();

    for (const auto& key : _storeQueue)
    {
      _dataSourceStoreListener(Get(key));
    }
    _storeQueue.clear();
  }

private:
  struct Record
  {
    std::atomic_bool available{false};
    std::mutex mutex;
    Data value;
  };

  std::unordered_set<Key> _retrieveQueue;
  std::unordered_set<Key> _storeQueue;
  std::unordered_map<Key, Record> _records{};

  DataSourceRetrieveListener _dataSourceRetrieveListener;
  DataSourceStoreListener _dataSourceStoreListener;
};

} // namespace soa

#endif //DATASTORAGE_HPP
