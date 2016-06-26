#include "storage/storage_server.h"

#include <leveldb/iterator.h>
#include <leveldb/status.h>

#include "interpreter/language_evaluation.h"
#include "interpreter/memory.h"
#include "util/source_util.h"

namespace steinlang {

namespace {

void InitEvalContext(Program* pgm, EvalContext* ctx) {
  AnnotateSource(pgm);
  *ctx->mutable_pgm() = *pgm;
  for (int i = pgm->stmt_size(); i-- > 0;) {
    *ctx->mutable_cur_ctx()->add_comp()->mutable_stmt() = pgm->stmt(i);
  }
}

}  // namespace

grpc::Status StorageServiceImpl::AddProgram(grpc::ServerContext* server_ctx,
                                            const AddProgramRequest* req,
                                            AddProgramResponse* resp) {
  Program pgm;
  // TODO parse req->text() into pgm.
  ScopedStorageItem<EvalContext> item = store_->GetStorageItem(req->key());
  leveldb::Status check_exists = item.Read();
  if (check_exists.ok()) {
    return grpc::Status(grpc::StatusCode::ABORTED, "key already exists");
  } else if (check_exists.IsNotFound()) {
    InitEvalContext(&pgm, item.mutable_value());
    leveldb::Status status = item.Write();
    if (!status.ok()) {
      return grpc::Status(grpc::StatusCode::INTERNAL,
                          "leveldb put failed: " + status.ToString());
    }
    return grpc::Status::OK;
  } else {
    return grpc::Status(grpc::StatusCode::INTERNAL,
                        "leveldb get failed: " + check_exists.ToString());
  }
}

grpc::Status StorageServiceImpl::ModifyProgram(grpc::ServerContext* server_ctx,
                                               const ModifyProgramRequest* req,
                                               ModifyProgramResponse* resp) {
  ScopedStorageItem<EvalContext> item = store_->GetStorageItem(req->key());
  leveldb::Status status = item.Read();
  if (!status.ok()) {
    return grpc::Status(grpc::StatusCode::INTERNAL,
                        "leveldb get failed: " + status.ToString());
  }

  const Program& old_pgm = item.value().pgm();
  Program new_pgm;
  // TODO parse req->text() into new_pgm.
  // TODO hotswap instead of destroying existing ctx.
  item.mutable_value()->Clear();
  InitEvalContext(&new_pgm, item.mutable_value());
  status = item.Write();
  if (!status.ok()) {
    return grpc::Status(grpc::StatusCode::INTERNAL,
                        "leveldb put failed: " + status.ToString());
  }
  return grpc::Status::OK;
}

grpc::Status StorageServiceImpl::ViewSnapshot(grpc::ServerContext* server_ctx,
                                              const ViewSnapshotRequest* req,
                                              ViewSnapshotResponse* resp) {
  ScopedStorageItem<EvalContext> item = store_->GetStorageItem(req->key());
  leveldb::Status status = item.Read();
  if (!status.ok()) {
    return grpc::Status(grpc::StatusCode::INTERNAL,
                        "leveldb get failed: " + status.ToString());
  }
  *resp->mutable_snapshot() = item.value();
  return grpc::Status::OK;
}

grpc::Status StorageServiceImpl::GetUpdates(
    grpc::ServerContext* server_ctx, const GetUpdatesRequest* req,
    grpc::ServerWriter<ProgramUpdate>* resp) {
  return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "not implemented");
}

leveldb::Status StorageServiceImpl::StepKey(const std::string& key,
                                            int num_steps,
                                            PoolingArenaAllocator* allocator) {
  ScopedStorageItem<EvalContext> item = store_->GetStorageItem(key);
  leveldb::Status status = item.Read();
  if (!status.ok()) {
    return status;
  }

  allocator->Reset();
  EvalContext* ctx = allocator->AllocateEvalContext();
  *ctx = item.value();
  Evaluator evaluator(ctx, allocator);
  for (int i = 0; i < num_steps && evaluator.HasComputation(); ++i) {
    evaluator.Step();
  }

  *item.mutable_value() = evaluator.ctx();
  return item.Write();
}

leveldb::Status StorageServiceImpl::StepAll(int num_steps) {
  auto it = store_->iterator();
  PoolingArenaAllocator allocator;
  leveldb::Status status;
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    leveldb::Status step_status =
        StepKey(it->key().ToString(), num_steps, &allocator);
    if (!step_status.ok()) {
      status = step_status;
    }
  }
  if (!it->status().ok()) {
    status = it->status();
  }
  return status;
}

}  // namespace steinlang
