#include <gflags/gflags.h>
#include <iostream>

#include "lang/interpreter/steinlang_parser.h"
#include "lang/interpreter/steinlang_syntax.pb.h"
#include "util/file_io.h"
#include "util/optional.h"
#include "util/proto/diff_proto.h"

namespace steinlang {
namespace {

template <typename Lexer, typename Parser>
util::Optional<typename Parser::ParseTreeNode> LexAndParse(
    const std::string& text, const Lexer& lexer, const Parser& parser) {
  const std::vector<typename Lexer::Token> tokens = lexer(text);
  auto parse_result = parser.Parse(tokens.begin(), tokens.end());

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

util::Optional<Program> ParseProgram(const std::string& text) {
  const util::Optional<steinlang::Parser::ParseTreeNode> parse_result =
      LexAndParse(text, steinlang::Tokenizer(), steinlang::Parser());
  if (!parse_result.is_present()) {
    return util::EmptyOptional();
  }
  return steinlang::ToProgram(parse_result.value());
}

}  // namespace
}  // namespace steinlang

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  if (argc != 3) {
    std::cout << "usage: " << argv[0] << " file1 file2\n";
    return 1;
  }

  util::Optional<steinlang::Program> lhs_pgm =
      steinlang::ParseProgram(util::ReadFileToString(argv[1]));
  if (!lhs_pgm.is_present()) {
    std::cout << "failed to parse " << argv[1] << ".\n";
    return 1;
  }
  util::Optional<steinlang::Program> rhs_pgm =
      steinlang::ParseProgram(util::ReadFileToString(argv[2]));
  if (!rhs_pgm.is_present()) {
    std::cout << "failed to parse " << argv[2] << ".\n";
    return 1;
  }

  const std::vector<util::ProtoDiffer::Modification> diffs =
      util::ProtoDiffer().Diff(&lhs_pgm.mutable_value(),
                               &rhs_pgm.mutable_value());
  for (const auto& m : diffs) {
    if (m.is_addition()) {
      std::cout << "Addition:\n";
    } else {
      std::cout << "Deletion:\n";
    }

    std::cout << "  parent:\n";
    std::cout << m.field.parent->DebugString();

    std::cout << "  parent_field:\n";
    std::cout << m.field.parent_field->DebugString();

    if (m.field.index >= 0) {
      std::cout << "  index: [" << m.field.index << "]\n";
    }
  }
  return 0;
}
