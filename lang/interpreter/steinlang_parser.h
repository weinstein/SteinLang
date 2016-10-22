#ifndef LANG_INTERPRETER_STEINLANG_PARSER_H_
#define LANG_INTERPRETER_STEINLANG_PARSER_H_

#include "lang/interpreter/steinlang_syntax.pb.h"
#include "lang/recursive_descent_parser.h"
#include "lang/tokenizer.h"

namespace steinlang {

enum class TokenTag {
  NONE,
  BOOL,
  INT,
  FLOAT,
  STRING,
  ID,
  COMMA,
  LPAREN,
  RPAREN,
  LBRACE,
  RBRACE,
  LBRACKET,
  RBRACKET,
  LAMBDA,
  COLON,
  EXCLAMATION_PT,
  MINUS,
  PLUS,
  STAR,
  FWD_SLASH,
  GE,
  GT,
  LE,
  LT,
  DBL_EQ,
  NE,
  DBL_AND,
  DBL_PIPE,
  IF,
  ELSE,
  RETURN,
  PRINT,
  WHILE,
  FOR,
  DEF,
  EQ,
  SEMICOLON,
  UNKNOWN,
};

inline std::ostream& operator<<(std::ostream& os, TokenTag tag) {
  os << (int) tag;
  return os;
}

class Tokenizer : public lang::Tokenizer<TokenTag> {
 public:
  Tokenizer();
};

enum class VariableTag {
  VARLIST,
  EXPLIST,
  TRAILER,
  TRAILER_LIST,
  LAMBDA_EXPR,
  ATOM,
  ATOM_EXPR,
  MONOP,
  BINOP,
  ARITH_EXPR,
  EXPR,
  BLOCK,
  STMT,
  PGM,
};

inline std::ostream& operator<<(std::ostream& os, VariableTag tag) {
  os << (int) tag;
  return os;
}

class Parser : public lang::RecursiveDescentParser<TokenTag, VariableTag> {
 public:
  Parser();
};

Program ToProgram(const Parser::ParseTreeNode& parse_tree);

}  // namespace steinlang

#endif  // LANG_INTERPRETER_STEINLANG_PARSER_H_
