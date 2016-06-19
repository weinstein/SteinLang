#include "storage/leveldb_store.h"

namespace steinlang {

namespace {

class OpImpl : public LevelDBStore::Op {
 public:
  OpImpl(const OpImpl&) = delete;
  OpImpl& operator=(const OpImpl&) = delete;

  OpImpl(const std::string& key, leveldb::DB* db, std::mutex* mu)
      : key_(key), db_(db), mu_(mu) {
    mu_->lock();
  }

  leveldb::Status CheckExistence() override {
    return db_->Get(leveldb::ReadOptions(), key_, &value_);
  }

  leveldb::Status Get(EvalContext* ctx) override {
    value_.clear();
    leveldb::Status status = db_->Get(leveldb::ReadOptions(), key_, &value_);
    if (!status.ok()) {
      return status;
    }
    if (!ctx->ParseFromString(value_)) {
      return leveldb::Status::Corruption("unparsable value for key", key_);
    }
    return leveldb::Status::OK();
  }

  leveldb::Status Put(const EvalContext& ctx) override {
    value_.clear();
    if (!ctx.SerializeToString(&value_)) {
      return leveldb::Status::InvalidArgument("unable to serialize EvalContext",
                                              ctx.ShortDebugString());
    }
    return db_->Put(leveldb::WriteOptions(), key_, value_);
  }

  ~OpImpl() override {
    mu_->unlock();
  }

 private:
  const std::string key_;
  std::string value_;
  leveldb::DB* db_;
  std::mutex* mu_;
};

}  // namespace

leveldb::Status LevelDBStore::Get(const std::string& key, EvalContext* ctx) {
  std::string value;
  leveldb::Status status = db_->Get(leveldb::ReadOptions(), key, &value);
  if (!status.ok()) {
    return status;
  }
  if (!ctx->ParseFromString(value)) {
    return leveldb::Status::Corruption("unparsable value for key", key);
  }
  return leveldb::Status::OK();
}

leveldb::Status LevelDBStore::Put(const std::string& key, const EvalContext& ctx) {
  std::string value;
  if (!ctx.SerializeToString(&value)) {
    return leveldb::Status::InvalidArgument("unable to serialize EvalContext",
                                            ctx.ShortDebugString());
  }
  return db_->Put(leveldb::WriteOptions(), key, value);
}

std::unique_ptr<LevelDBStore::Op> LevelDBStore::LockForOp(
    const std::string& key) {
  std::mutex* mu = &(key_locks_[key]);
  return std::make_unique<OpImpl>(key, db_.get(), mu);
}

}  // namespace steinlang
