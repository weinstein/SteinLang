#ifndef STORAGE_LEVELDB_STORE_H_
#define STORAGE_LEVELDB_STORE_H_

#include <google/protobuf/message.h>
#include <leveldb/db.h>
#include <leveldb/status.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "proto/language.pb.h"
#include "util/lru_cache.h"

namespace steinlang {

leveldb::Status GetProto(leveldb::DB* db, const std::string& key,
                         google::protobuf::Message* msg);

leveldb::Status PutProto(leveldb::DB* db, const std::string& key,
                         const google::protobuf::Message& msg);

// A google::protobuf::Message item in LevelDB storage which corresponds to a
// given leveldb key.
// The StorageItem can be read, written, and locked for exclusive access.
// Concurrent access to a StorageItem is NOT thread safe; accesses should be
// guarded with unique_lock().
template <typename T>
class StorageItem {
 public:
  StorageItem(leveldb::DB* db, const std::string& key)
      : db_(*db), key_(key), mu_(std::make_unique<std::mutex>()) {}

  StorageItem(const StorageItem&) = delete;
  StorageItem<T>& operator=(const StorageItem<T>&) = delete;

  StorageItem(StorageItem&&) = default;
  StorageItem& operator=(StorageItem&&) = default;

  leveldb::Status Read() {
    return GetProto(&db_, key_, &value_);
  }

  leveldb::Status Write() {
    return PutProto(&db_, key_, value_);
  }

  const std::string& key() const { return key_; }

  const T& value() const { return value_; }
  T* mutable_value() { return &value_; }

  std::unique_lock<std::mutex> unique_lock() {
    return std::unique_lock<std::mutex>(*mu_);
  }

 private:
  leveldb::DB& db_;
  const std::string key_;
  T value_;
  std::unique_ptr<std::mutex> mu_;
  bool is_synced_;
};

// Pretty much the same as StorageItem<T>, but safely locked for concurrent
// access.
template <typename T>
class ScopedStorageItem {
 public:
  explicit ScopedStorageItem(std::shared_ptr<StorageItem<T>> item)
      : item_(item), lock_(item_->unique_lock()) {}

  leveldb::Status Read() { return item_->Read(); }
  leveldb::Status Write() { return item_->Write(); }

  const std::string& key() const { return item_->key(); }
  const T& value() const { return item_->value(); }
  T* mutable_value() { return item_->mutable_value(); }

 private:
  std::shared_ptr<StorageItem<T>> item_;
  std::unique_lock<std::mutex> lock_;
};

// A wrapper around leveldb for storing protobuf messages of type T.
// The values are stored in-memory in an LRU cache and can be read/written to
// leveldb at any time.
// Access is given to the value for each key exclusively, allowing
// read-modify-write patterns.
// Thread safe.
// Intended usage is along these lines:
//
// LevelDBStore::Options options;
// options.max_size = 1024;
// auto db = std::make_unique<leveldb::DB>(...);
// LevelDBStore store(options, std::move(db));
//
// // In some thread
// {
//   ScopedStorageItem<T> item = store.GetStorageItem("foo");
//   leveldb::Status status = item.Read();
//   if (item.value().bar()) {
//     item.mutable_value()->set_bar(false);
//     status = item.Write();
//   }
// }
//
// // In some other thread
// {
//   ScopedStorageItem<T> item = store.GetStorageItem("foo");
//   leveldb::Status status = item.Read();
//   if (!item.value().bar()) {
//     item.mutable_value()->set_baz("abc");
//     status = item.Write();
//   }
// }

struct LevelDBStoreOptions {
  // Max LRU Cache size
  size_t max_size = 1 << 24;
};

template <typename T>
class LevelDBStore {
 public:
  explicit LevelDBStore(const LevelDBStoreOptions& options,
                        std::unique_ptr<leveldb::DB> db)
      : db_(std::move(db)), cache_(options.max_size) {}

  ScopedStorageItem<T> GetStorageItem(const std::string& key) {
    std::lock_guard<std::mutex> cache_lock(cache_mu_);
    return ScopedStorageItem<T>(cache_.FindOrEmplace(
        key, std::make_shared<StorageItem<T>>(db_.get(), key)));
  }

 private:
  std::unique_ptr<leveldb::DB> db_;
  LRUCache<std::string, std::shared_ptr<StorageItem<T>>> cache_;
  std::mutex cache_mu_;
};

}  // namespace steinlang

#endif  // STORAGE_LEVELDB_STORE_H_
