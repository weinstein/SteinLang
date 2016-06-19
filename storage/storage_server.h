#ifndef STORAGE_STORAGE_SERVER_H_
#define STORAGE_STORAGE_SERVER_H_

#include <grpc++/server.h>
#include <grpc++/server_context.h>
#include <grpc/grpc.h>
#include <leveldb/db.h>
#include <memory>

#include "proto/service.grpc.pb.h"

namespace steinlang {

class StorageServiceImpl final : public Storage::Service {
 public:
  explicit StorageServiceImpl(std::unique_ptr<leveldb::DB> db)
      : db_(std::move(db)) {}

  grpc::Status AddProgram(grpc::ServerContext* server_ctx,
                          const AddProgramRequest* req,
                          AddProgramResponse* resp) override;

  grpc::Status ModifyProgram(grpc::ServerContext* server_ctx,
                             const ModifyProgramRequest* req,
                             ModifyProgramResponse* resp) override;

  grpc::Status ViewSnapshot(grpc::ServerContext* server_ctx,
                            const ViewSnapshotRequest* req,
                            ViewSnapshotResponse* resp) override;

  grpc::Status GetUpdates(grpc::ServerContext* server_ctx,
                          const GetUpdatesRequest* req,
                          grpc::ServerWriter<ProgramUpdate>* resp) override;

 private:
  std::unique_ptr<leveldb::DB> db_;
};

}  // namespace steinlang

#endif  // STORAGE_STORAGE_SERVER_H_
