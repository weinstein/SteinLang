#include <gflags/gflags.h>
#include <grpc++/server_builder.h>
#include <grpc++/security/server_credentials.h>
#include <leveldb/db.h>
#include <memory>
#include <stdio.h>
#include <string>

#include "storage/leveldb_store.h"
#include "storage/storage_server.h"

DEFINE_string(address, "0.0.0.0:8000", "Server address");
DEFINE_string(db_path, "/tmp/db", "leveldb storage path");

namespace {

std::unique_ptr<steinlang::LevelDBStore> OpenLevelDBOrDie(
    const std::string& db_path) {
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::DB* db;
  assert(leveldb::DB::Open(options, db_path, &db).ok());
  printf("Opened leveldb %s\n", FLAGS_db_path.c_str());
  return std::make_unique<steinlang::LevelDBStore>(
      std::unique_ptr<leveldb::DB>(db));
}

}  // namespace

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  steinlang::StorageServiceImpl service(OpenLevelDBOrDie(FLAGS_db_path));
  grpc::ServerBuilder builder;
  builder.AddListeningPort(FLAGS_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  printf("Server started on %s\n", FLAGS_address.c_str());

  server->Wait();
  return 0;
}
