#ifndef STORAGE_LEVELDB_STORE_H_
#define STORAGE_LEVELDB_STORE_H_

#include <leveldb/db.h>
#include <leveldb/status.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "proto/language.pb.h"

namespace steinlang {

class LevelDBStore {
 public:
  class Op {
   public:
    Op() = default;
    virtual ~Op() = default;
    virtual leveldb::Status CheckExistence() = 0;
    virtual leveldb::Status Get(EvalContext* ctx) = 0;
    virtual leveldb::Status Put(const EvalContext& ctx) = 0;
  };

  explicit LevelDBStore(std::unique_ptr<leveldb::DB> db) : db_(std::move(db)) {}

  leveldb::Status Get(const std::string& key, EvalContext* ctx);

  leveldb::Status Put(const std::string& key, const EvalContext& ctx);

  std::unique_ptr<Op> LockForOp(const std::string& key);

 private:
  std::unique_ptr<leveldb::DB> db_;
  std::map<std::string, std::mutex> key_locks_;
};

}  // namespace steinlang

#endif  // STORAGE_LEVELDB_STORE_H_
