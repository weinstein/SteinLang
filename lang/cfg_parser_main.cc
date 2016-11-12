#include <gflags/gflags.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "lang/cfg_parser.h"
#include "util/file_io.h"
#include "util/optional.h"

DEFINE_string(grammar_def, "", "file containing the grammar definition");
DEFINE_string(start_symbol, "start",
              "start symbol (production rule name) for parsing input");
DEFINE_string(ignore_regex, "",
              "if non empty, ignore this regex during tokenization of input");
DEFINE_bool(debug_print_parse_trees, false,
            "if true, print parse tree DebugStrings after parsing grammars "
            "definitions or inputs");

namespace {

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

}

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);

  lang::CfgTokenizer cfg_tokenizer;
  lang::CfgParser cfg_parser;
  const util::Optional<lang::CfgParser::ParseTreeNode> cfg_parse_result =
      LexAndParse(util::ReadFileToString(FLAGS_grammar_def), cfg_tokenizer,
                  cfg_parser);
  if (!cfg_parse_result.is_present()) {
    std::cout << "failed to parse grammar definition.\n";
    return 1;
  }

  lang::ParsedGrammar parsed_grammar(FLAGS_start_symbol,
                                     cfg_parse_result.value());
  if (!FLAGS_ignore_regex.empty()) {
    parsed_grammar.tokenizer.ignore_rule(FLAGS_ignore_regex);
  }
  const util::Optional<lang::ParsedGrammar::Parser::ParseTreeNode>
      input_parse_result =
          LexAndParse(util::ReadStdInToString(), parsed_grammar.tokenizer,
                      parsed_grammar.parser);
  if (!input_parse_result.is_present()) {
    std::cout << "failed to parse input.\n";
    return 1;
  }

  return 0;
}
