#include <chrono>
#include <gflags/gflags.h>
#include <stdio.h>

#include "file_io.h"
#include "language_evaluation.h"
#include "map_util.h"

DEFINE_string(
    input_file, "input.txt",
    "ASCII text format Program protobuf to run. See proto/language.proto.");

DEFINE_bool(debug_print_steps, false,
            "If true, print verbose evaluation state after each step. This "
            "slows down execution significantly.");

namespace steinlang {

void DebugPrint(const EvalContext& ctx) {
  printf("================\n");
  printf("results: .");
  for (const Result& r : ctx.cur_ctx().result()) {
    printf(" <~ %s", r.ShortDebugString().c_str());
  }
  printf("\n--------\n");
  printf("env:\n");
  for (const auto& kv : ctx.cur_ctx().env()) {
    printf("  env[%s] = %s\n", kv.first.c_str(),
           ctx.store(kv.second).ShortDebugString().c_str());
  }
  printf("\n--------\n");
  printf("comps: .");
  for (const Computation c : ctx.cur_ctx().comp()) {
    printf(" <~ %s", c.ShortDebugString().c_str());
  }
  printf("\n--------\n");
}

int evaluate(std::unique_ptr<Evaluator> evaluator) {
  int steps = 0;
  int num_output_printed = 0;
  while (evaluator->HasComputation()) {
    if (FLAGS_debug_print_steps) {
      DebugPrint(evaluator->ctx());
    }
    evaluator->Step();
    if (evaluator->ctx().output_size() > num_output_printed) {
      printf("output: %s\n",
             evaluator->ctx().output(num_output_printed++).c_str());
    }
    ++steps;
  }
  return steps;
}

void CtxFromFile(const std::string& fname, EvalContext* ctx) {
  Program pgm = ParseAsciiProgram(fname);
  *ctx->mutable_pgm() = pgm;
  for (int i = pgm.stmt_size(); i-- > 0;) {
    *ctx->mutable_cur_ctx()->add_comp()->mutable_stmt() = pgm.stmt(i);
  }
}

}  // namespace steinlang

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  steinlang::PoolingArenaAllocator allocator;
  steinlang::EvalContext* ctx = allocator.AllocateEvalContext();
  steinlang::CtxFromFile(FLAGS_input_file, ctx);
  std::unique_ptr<steinlang::Evaluator> evaluator(
      new steinlang::Evaluator(ctx, &allocator));
  auto start = std::chrono::high_resolution_clock::now();
  int num_steps = evaluate(std::move(evaluator));
  auto elapsed = std::chrono::high_resolution_clock::now() - start;
  long long microseconds =
      std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
  printf("total num steps evaluated: %d\n", num_steps);
  printf("total time: %lld us\n", microseconds);
  printf("avg: %f us / step\n", static_cast<float>(microseconds) / num_steps);
  return 0;
}
