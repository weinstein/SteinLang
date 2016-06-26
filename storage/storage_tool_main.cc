#include <gflags/gflags.h>
#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>
#include <stdio.h>

#include "proto/service.grpc.pb.h"

DEFINE_string(address, "0.0.0.0:8000", "Server address");
DEFINE_string(key, "", "Key for AddProgramRequest");

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  std::shared_ptr<grpc::Channel> channel =
      grpc::CreateChannel(FLAGS_address, grpc::InsecureChannelCredentials());
  auto stub = steinlang::Storage::NewStub(channel);
  grpc::ClientContext ctx;
  steinlang::AddProgramRequest req;
  req.set_key(FLAGS_key);
  steinlang::AddProgramResponse resp;
  grpc::Status status = stub->AddProgram(&ctx, req, &resp);
  printf("AddProgramRequest: %s\n", req.ShortDebugString().c_str());
  printf("AddProgramResponse: %s\n", resp.ShortDebugString().c_str());
  printf("status: %d %s\n", status.error_code(),
         status.error_message().c_str());
}
