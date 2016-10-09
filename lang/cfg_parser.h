#ifndef LANG_CFG_PARSER_H_
#define LANG_CFG_PARSER_H_

#include <map>
#include <string>

#include "lang/tokenizer.h"
#include "lang/recursive_descent_parser.h"

namespace lang {

enum class CfgToken {
  INT,
  ID,
  REGEX_TOK,
  LITERAL_TOK,
  PIPE,
  SEMICOLON,
  PRODUCES,
  LPAREN,
  RPAREN,
  LBRACKET,
  RBRACKET,
  LBRACE,
  RBRACE,
  COMMA,
  STAR,
  PLUS,
  QMARK,
  UNKNOWN,
};

inline std::ostream& operator<<(std::ostream& os, CfgToken x) {
  os << (int) x;
  return os;
}

class CfgTokenizer : public Tokenizer<CfgToken> {
 public:
  CfgTokenizer();

 private:
};

enum class CfgSyntax {
  INT,
  ID,
  REGEX_TOK,
  LITERAL_TOK,
  TERM,
  CARDINALITY,
  EXPR,
  EXPRS,
  ALTERNATIVES,
  RULE,
  GRAMMAR,
};

inline std::ostream& operator<<(std::ostream& os, CfgSyntax x) {
  os << (int) x;
  return os;
}

class CfgParser : public RecursiveDescentParser<CfgToken, CfgSyntax> {
 public:
  CfgParser();

 private:
};

struct ParsedGrammar {
  Tokenizer<std::string> tokenizer;

  typedef RecursiveDescentParser<std::string, std::string> Parser;
  Parser parser;

  ParsedGrammar(std::string start_rule,
                const CfgParser::ParseTreeNode& parse_tree);

 private:
  ParsedGrammar(std::string start_rule) : parser(std::move(start_rule)) {}
};

}  // namespace lang

#endif  // LANG_CFG_PARSER_H_
