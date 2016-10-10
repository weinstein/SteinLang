#include "lang/interpreter/steinlang_parser.h"

namespace steinlang {

Tokenizer::Tokenizer() {
  add_rule(TokenTag::COMMA, ",");
  add_rule(TokenTag::LPAREN, R"(\()");
  add_rule(TokenTag::RPAREN, R"(\))");
  add_rule(TokenTag::LBRACKET, R"(\[)");
  add_rule(TokenTag::RBRACKET, R"(\])");
  add_rule(TokenTag::LBRACE, R"(\{)");
  add_rule(TokenTag::RBRACE, R"(\})");
  add_rule(TokenTag::LAMBDA, "lambda");
  add_rule(TokenTag::COLON, ":");
  add_rule(TokenTag::EXCLAMATION_PT, "!");
  add_rule(TokenTag::MINUS, "-");
  add_rule(TokenTag::PLUS, R"(\+)");
  add_rule(TokenTag::STAR, R"(\*)");
  add_rule(TokenTag::FWD_SLASH, "/");
  add_rule(TokenTag::GE, ">=");
  add_rule(TokenTag::GT, ">");
  add_rule(TokenTag::LE, "<=");
  add_rule(TokenTag::LT, "<");
  add_rule(TokenTag::DBL_EQ, "==");
  add_rule(TokenTag::NE, "!=");
  add_rule(TokenTag::DBL_AND, "&&");
  add_rule(TokenTag::DBL_PIPE, R"(\|\|)");
  add_rule(TokenTag::IF, "if");
  add_rule(TokenTag::ELSE, "else");
  add_rule(TokenTag::RETURN, "return");
  add_rule(TokenTag::PRINT, "print");
  add_rule(TokenTag::EQ, "=");
  add_rule(TokenTag::SEMICOLON, ";");
  add_rule(TokenTag::NONE, "None");
  add_rule(TokenTag::BOOL, "(True|False)");
  add_rule(TokenTag::INT, R"(\d+)");
  add_rule(TokenTag::FLOAT, R"(\d+\.\d*)");
  add_rule(TokenTag::STRING, R"("(\\.|[^"])*")");
  add_rule(TokenTag::ID, R"([[:alpha:]_]\w*)");
  add_rule(TokenTag::UNKNOWN, ".");
  ignore_rule(R"(#[^\n]*)");
  ignore_rule(R"(\s+)");
}

Parser::Parser()
    : lang::RecursiveDescentParser<TokenTag, VariableTag>(VariableTag::PGM) {
  add_rule(VariableTag::VARLIST,
           {Term::Terminal(TokenTag::ID),
            {Cardinality::Any(), Term::Group({Term::Terminal(TokenTag::COMMA),
                                              Term::Terminal(TokenTag::ID)})}});
  add_rule(
      VariableTag::EXPLIST,
      {{Cardinality::AtMost(1),
        Term::Group({Term::NonTerminal(VariableTag::EXPR),
                     {Cardinality::Any(),
                      Term::Group({Term::Terminal(TokenTag::COMMA),
                                   Term::NonTerminal(VariableTag::EXPR)})}})}});
  add_rule(VariableTag::TRAILER, {Term::Terminal(TokenTag::LPAREN),
                                  Term::NonTerminal(VariableTag::EXPLIST),
                                  Term::Terminal(TokenTag::RPAREN)});

  add_rule(VariableTag::LAMBDA_EXPR, {Term::Terminal(TokenTag::LAMBDA),
                                      Term::NonTerminal(VariableTag::VARLIST),
                                      Term::Terminal(TokenTag::COLON),
                                      Term::NonTerminal(VariableTag::EXPR)});

  add_rule(VariableTag::ATOM, {Term::Terminal(TokenTag::LPAREN),
                               Term::NonTerminal(VariableTag::EXPR),
                               Term::Terminal(TokenTag::RPAREN)});
  add_rule(VariableTag::ATOM, {Term::Terminal(TokenTag::NONE)});
  add_rule(VariableTag::ATOM, {Term::Terminal(TokenTag::BOOL)});
  add_rule(VariableTag::ATOM, {Term::Terminal(TokenTag::INT)});
  add_rule(VariableTag::ATOM, {Term::Terminal(TokenTag::FLOAT)});
  add_rule(VariableTag::ATOM, {Term::Terminal(TokenTag::STRING)});
  add_rule(VariableTag::ATOM, {Term::Terminal(TokenTag::ID)});

  add_rule(VariableTag::ATOM_EXPR,
           {Term::NonTerminal(VariableTag::ATOM),
            {Cardinality::Any(), Term::NonTerminal(VariableTag::TRAILER)}});

  add_rule(VariableTag::MONOP, {Term::Terminal(TokenTag::EXCLAMATION_PT)});
  add_rule(VariableTag::MONOP, {Term::Terminal(TokenTag::MINUS)});

  add_rule(VariableTag::BINOP, {Term::Terminal(TokenTag::PLUS)});
  add_rule(VariableTag::BINOP, {Term::Terminal(TokenTag::MINUS)});
  add_rule(VariableTag::BINOP, {Term::Terminal(TokenTag::STAR)});
  add_rule(VariableTag::BINOP, {Term::Terminal(TokenTag::FWD_SLASH)});
  add_rule(VariableTag::BINOP, {Term::Terminal(TokenTag::GE)});
  add_rule(VariableTag::BINOP, {Term::Terminal(TokenTag::GT)});
  add_rule(VariableTag::BINOP, {Term::Terminal(TokenTag::LE)});
  add_rule(VariableTag::BINOP, {Term::Terminal(TokenTag::LT)});
  add_rule(VariableTag::BINOP, {Term::Terminal(TokenTag::DBL_EQ)});
  add_rule(VariableTag::BINOP, {Term::Terminal(TokenTag::NE)});
  add_rule(VariableTag::BINOP, {Term::Terminal(TokenTag::DBL_AND)});
  add_rule(VariableTag::BINOP, {Term::Terminal(TokenTag::DBL_PIPE)});

  add_rule(VariableTag::ARITH_EXPR,
           {Term::NonTerminal(VariableTag::MONOP),
            Term::NonTerminal(VariableTag::ATOM_EXPR)});
  add_rule(VariableTag::ARITH_EXPR,
           {Term::NonTerminal(VariableTag::ATOM_EXPR),
            Term::NonTerminal(VariableTag::BINOP),
            Term::NonTerminal(VariableTag::ATOM_EXPR)});
  add_rule(VariableTag::ARITH_EXPR,
           {Term::NonTerminal(VariableTag::ATOM_EXPR)});

  add_rule(VariableTag::EXPR, {Term::Terminal(TokenTag::LAMBDA),
                               Term::NonTerminal(VariableTag::VARLIST),
                               Term::Terminal(TokenTag::COLON),
                               Term::NonTerminal(VariableTag::EXPR)});
  add_rule(VariableTag::EXPR, {Term::NonTerminal(VariableTag::ARITH_EXPR),
                               Term::Terminal(TokenTag::IF),
                               Term::NonTerminal(VariableTag::ARITH_EXPR),
                               Term::Terminal(TokenTag::ELSE),
                               Term::NonTerminal(VariableTag::ARITH_EXPR)});
  add_rule(VariableTag::EXPR, {Term::NonTerminal(VariableTag::ARITH_EXPR)});

  add_rule(VariableTag::STMT, {Term::Terminal(TokenTag::RETURN),
                               Term::NonTerminal(VariableTag::EXPR),
                               Term::Terminal(TokenTag::SEMICOLON)});
  add_rule(VariableTag::STMT, {Term::Terminal(TokenTag::PRINT),
                               Term::NonTerminal(VariableTag::EXPR),
                               Term::Terminal(TokenTag::SEMICOLON)});
  add_rule(VariableTag::STMT, {Term::NonTerminal(VariableTag::EXPR),
                               Term::Terminal(TokenTag::SEMICOLON)});
  add_rule(VariableTag::STMT,
           {Term::NonTerminal(VariableTag::EXPR), Term::Terminal(TokenTag::EQ),
            Term::NonTerminal(VariableTag::EXPR),
            Term::Terminal(TokenTag::SEMICOLON)});

  add_rule(VariableTag::PGM,
           {{Cardinality::AtLeast(1), Term::NonTerminal(VariableTag::STMT)}});
}

}  // namespace steinlang
