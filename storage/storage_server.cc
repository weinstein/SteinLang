#include "storage/storage_server.h"

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
  std::string value;
  leveldb::Status s = db_->Get(leveldb::ReadOptions(), req->key(), &value);
  if (s.ok() && !value.empty()) {
    return grpc::Status(grpc::StatusCode::ABORTED, "key already exists");
  } else if (!s.IsNotFound()) {
    return grpc::Status(grpc::StatusCode::INTERNAL, "leveldb read failed");
  }

  Program pgm;
  // TODO parse req->text() into pgm.
  EvalContext ctx;
  InitEvalContext(&pgm, &ctx);
  if (!ctx.SerializeToString(&value)) {
    return grpc::Status(grpc::StatusCode::INTERNAL, "serialization failed");
  }

  s = db_->Put(leveldb::WriteOptions(), req->key(), value);
  if (!s.ok()) {
    return grpc::Status(grpc::StatusCode::INTERNAL,
                        "leveldb write failed: " + s.ToString());
  }
  return grpc::Status::OK;
}

grpc::Status StorageServiceImpl::ModifyProgram(
    grpc::ServerContext* server_ctx, const ModifyProgramRequest* req,
    ModifyProgramResponse* resp) {
  std::string value;
  leveldb::Status s = db_->Get(leveldb::ReadOptions(), req->key(), &value);
  if (s.IsNotFound()) {
    return grpc::Status(grpc::StatusCode::NOT_FOUND, "key not found");
  } else if (!s.ok()) {
    return grpc::Status(grpc::StatusCode::INTERNAL, "leveldb read failed");
  }

  EvalContext ctx;
  if (!ctx.ParseFromString(value)) {
    return grpc::Status(grpc::StatusCode::INTERNAL, "parse failed");
  }
  const Program& old_pgm = ctx.pgm();
  Program new_pgm;
  // TODO parse req->text() into new_pgm.
  // TODO hotswap instead of destroying existing ctx.
  old_ctx.Clear();
  InitEvalContext(&new_pgm, &ctx);
  if (!ctx.SerializeToString(&value)) {
    return grpc::Status(grpc::StatusCode::INTERNAL, "serialization failed");
  }

  s = db_->Put(leveldb::WriteOptions(), req->key(), value);
  if (!s.ok()) {
    return grpc::Status(grpc::StatusCode::INTERNAL,
                        "leveldb write failed: " + s.ToString());
  }
  return grpc::Status::OK;
}

grpc::Status StorageServiceImpl::ViewSnapshot(
    grpc::ServerContext* server_ctx, const ViewSnapshotRequest* req,
    ViewSnapshotResponse* resp) {
  std::string value;
  leveldb::Status s = db_->Get(leveldb::ReadOptions(), req->key(), &value);
  if (!s.ok()) {
    return grpc::Status(grpc::StatusCode::INTERNAL, "leveldb read failed");
  }
  EvalContext ctx;
  if (!ctx.ParseFromString(value)) {
    return grpc::Status(grpc::StatusCode::INTERNAL, "parse failed");
  }
  *resp->mutable_snapshot() = ctx;
  return grpc::Status::OK;
}

grpc::Status StorageServiceImpl::GetUpdates(
    grpc::ServerContext* server_ctx, const GetUpdatesRequest* req,
    grpc::ServerWriter<ProgramUpdate>* resp) {
  return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "not implemented");
}

}  // namespace steinlang
