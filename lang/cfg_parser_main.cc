#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "lang/cfg_parser.h"

namespace {

std::string ReadStdIn() {
  std::stringstream ss;
  ss << std::cin.rdbuf();
  return ss.str();
}

std::string ReadFile(const std::string& fname) {
  std::ifstream fs(fname);
  std::stringstream ss;
  ss << fs.rdbuf();
  return ss.str();
}

}

int main(int argc, char** argv) {
  lang::CfgTokenizer tokenizer;
  auto tokens = tokenizer(ReadStdIn());
  for (const auto& token : tokens) {
    if (token.tag == lang::CfgToken::UNKNOWN) {
      std::cout << "unknown token at position " << token.pos << "\n";

      auto start_pos = token.pos - 10;
      if (start_pos < 0) start_pos = 0;

      auto end_pos = token.pos + 10;
      if (end_pos > tokens.size()) end_pos = tokens.size();

      for (auto it = tokens.begin() + start_pos; it != tokens.begin() + end_pos; ++it) {
        std::cout << " " << it->value;
      }
      std::cout << "\n";
      for (auto it = tokens.begin() + start_pos; it != tokens.begin() + end_pos; ++it) {
        char fill = ' ';
        if (it == tokens.begin() + token.pos) {
          fill = '^';
        }
        std::cout << " " << std::string(it->value.size(), fill);
      }
      std::cout << "\n";
      return 1;
    }
  }

  lang::CfgParser parser;
  auto parse_result = parser.Parse(tokens.begin(), tokens.end());
  std::cout << parse_result.DebugString(tokens.end()) << "\n";

  std::vector<lang::CfgTokenizer::Token> unparsed_tokens(parse_result.pos,
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
    return false;
  }

  return 0;
}
