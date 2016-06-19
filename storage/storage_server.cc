#include "storage/storage_server.h"

#include <leveldb/status.h>

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
  auto op = store_->LockForOp(req->key());
  leveldb::Status check_exists = op->CheckExistence();
  if (check_exists.ok()) {
    return grpc::Status(grpc::StatusCode::ABORTED, "key already exists");
  } else if (check_exists.IsNotFound()) {
    Program pgm;
    // TODO parse req->text() into pgm.
    EvalContext ctx;
    InitEvalContext(&pgm, &ctx);
    leveldb::Status status = op->Put(ctx);
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
  auto op = store_->LockForOp(req->key());
  EvalContext ctx;
  leveldb::Status status = op->Get(&ctx);
  if (!status.ok()) {
    return grpc::Status(grpc::StatusCode::INTERNAL,
                        "leveldb get failed: " + status.ToString());
  }

  const Program& old_pgm = ctx.pgm();
  Program new_pgm;
  // TODO parse req->text() into new_pgm.
  // TODO hotswap instead of destroying existing ctx.
  ctx.Clear();
  InitEvalContext(&new_pgm, &ctx);
  status = op->Put(ctx);
  if (!status.ok()) {
    return grpc::Status(grpc::StatusCode::INTERNAL,
                        "leveldb put failed: " + status.ToString());
  }
  return grpc::Status::OK;
}

grpc::Status StorageServiceImpl::ViewSnapshot(grpc::ServerContext* server_ctx,
                                              const ViewSnapshotRequest* req,
                                              ViewSnapshotResponse* resp) {
  EvalContext ctx;
  leveldb::Status status = store_->Get(req->key(), &ctx);
  if (!status.ok()) {
    return grpc::Status(grpc::StatusCode::INTERNAL,
                        "leveldb get failed: " + status.ToString());
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
