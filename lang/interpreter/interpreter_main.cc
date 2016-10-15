#include <chrono>
#include <gflags/gflags.h>
#include <iostream>
#include <stdio.h>

#include "lang/interpreter/language_evaluation.h"
#include "lang/interpreter/source_util.h"
#include "lang/interpreter/steinlang_parser.h"
#include "util/file_io.h"
#include "util/optional.h"

DEFINE_bool(debug_print_parse_trees, false, "");
DEFINE_bool(debug_print_syntax_tree, false, "");
DEFINE_bool(debug_print_steps, false,
            "If true, print verbose evaluation state after each step. This "
            "slows down execution significantly.");
DEFINE_bool(debug_print_timing, false, "");

namespace {

std::string ReadStdIn() {
  std::stringstream ss;
  ss << std::cin.rdbuf();
  return ss.str();
}

template <typename Lexer, typename Parser>
util::Optional<typename Parser::ParseTreeNode> LexAndParse(
    const std::string& text, const Lexer& lexer, const Parser& parser) {
  const std::vector<typename Lexer::Token> tokens = lexer(text);
  auto parse_result = parser.Parse(tokens.begin(), tokens.end());

  if (FLAGS_debug_print_parse_trees) {
    std::cout << parse_result.DebugString(tokens.end()) << "\n";
  }

  const std::vector<typename Lexer::Token> unparsed_tokens(parse_result.pos,
                                                           tokens.end());
  if (!unparsed_tokens.empty()) {
    std::cout << "unparsed tokens:";
    for (const auto& token : unparsed_tokens) {
      std::cout << " " << token.value;
    }
    std::cout << "\n";
  }

  if (!parse_result.success) {
    std::cout << "parse failed.\n";
    return util::EmptyOptional();
  }
  return std::move(parse_result.node);
}

}  // namespace

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
    evaluator->Step();
    for (const std::string& output : evaluator->consume_output()) {
      printf("output: %s\n", output.c_str());
    }
    ++steps;
    if (FLAGS_debug_print_steps) {
      DebugPrint(evaluator->ctx());
    }
  }
  return steps;
}

void InitCtx(const Program& pgm, EvalContext* ctx) {
  *ctx->mutable_pgm() = pgm;
  AnnotateSource(ctx->mutable_pgm());
  for (int i = pgm.stmt_size(); i-- > 0;) {
    *ctx->mutable_cur_ctx()->add_comp()->mutable_stmt() = pgm.stmt(i);
  }
}

bool Evaluate(const std::string& input, EvalContext* ctx, PoolingArenaAllocator* allocator) {
  const util::Optional<steinlang::Parser::ParseTreeNode> parse_result =
      LexAndParse(input, steinlang::Tokenizer(), steinlang::Parser());
  if (!parse_result.is_present()) {
    std::cout << "failed to parse input.\n";
    return false;
  }

  steinlang::Program pgm = steinlang::ToProgram(parse_result.value());
  if (FLAGS_debug_print_syntax_tree) {
    std::cout << pgm.DebugString() << "\n";
  }

  steinlang::InitCtx(pgm, ctx);
  auto evaluator = std::make_unique<steinlang::Evaluator>(ctx, allocator);
  auto start = std::chrono::high_resolution_clock::now();
  int num_steps = evaluate(std::move(evaluator));
  auto elapsed = std::chrono::high_resolution_clock::now() - start;
  long long microseconds =
      std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
  if (FLAGS_debug_print_timing) {
    printf("total num steps evaluated: %d\n", num_steps);
    printf("total time: %lld us\n", microseconds);
    printf("avg: %f us / step\n", static_cast<float>(microseconds) / num_steps);
  }
  return true;
}

}  // namespace steinlang

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);

  steinlang::PoolingArenaAllocator allocator;
  steinlang::EvalContext* ctx = allocator.AllocateEvalContext();
  for (std::string line; std::getline(std::cin, line) && Evaluate(line, ctx, &allocator);) {}

  return 0;
}
