#include "storage/leveldb_store.h"

namespace steinlang {

leveldb::Status GetProto(leveldb::DB* db, const std::string& key,
                         google::protobuf::Message* msg) {
  std::string value;
  leveldb::Status status = db->Get(leveldb::ReadOptions(), key, &value);
  if (!status.ok()) {
    return status;
  }
  if (!msg->ParseFromString(value)) {
    return leveldb::Status::Corruption("unparsable value for key", key);
  }
  return leveldb::Status::OK();
}

leveldb::Status PutProto(leveldb::DB* db, const std::string& key,
                         const google::protobuf::Message& msg) {
  std::string value;
  if (!msg.SerializeToString(&value)) {
    return leveldb::Status::InvalidArgument("unable to serialize EvalContext",
                                            msg.ShortDebugString());
  }
  return db->Put(leveldb::WriteOptions(), key, value);
}

}  // namespace steinlang
