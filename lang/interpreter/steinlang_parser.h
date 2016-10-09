#ifndef LANG_INTERPRETER_STEINLANG_PARSER_H_
#define LANG_INTERPRETER_STEINLANG_PARSER_H_

#include "lang/tokenizer.h"
#include "lang/recursive_descent_parser.h"

namespace steinlang {

enum class Token {
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
  EQ,
  SEMICOLON,
  UNKNOWN,
};

class Tokenizer : public lang::Tokenizer<Token> {
 public:
  Tokenizer();
};

enum class Variable {
  VARLIST,
  EXPLIST,
  TRAILER,
  LAMBDA_EXPR,
  ATOM,
  ATOM_EXPR,
  MONOP,
  BINOP,
  ARITH_EXPR,
  EXPR,
  STMT,
  PGM,
};

class Parser : public lang::RecursiveDescentParser<Token, Variable> {
 public:
  Parser();
};

}  // namespace steinlang

#endif  // LANG_INTERPRETER_STEINLANG_PARSER_H_
