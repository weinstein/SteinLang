#include "lang/interpreter/steinlang_parser.h"

namespace steinlang {

Tokenizer::Tokenizer() {
  add_rule(Token::COMMA, ",");
  add_rule(Token::LPAREN, R"(\()");
  add_rule(Token::RPAREN, R"(\))");
  add_rule(Token::LBRACKET, R"(\[)");
  add_rule(Token::RBRACKET, R"(\])");
  add_rule(Token::LBRACE, R"(\{)");
  add_rule(Token::RBRACE, R"(\})");
  add_rule(Token::LAMBDA, "lambda");
  add_rule(Token::COLON, ":");
  add_rule(Token::EXCLAMATION_PT, "!");
  add_rule(Token::MINUS, "-");
  add_rule(Token::PLUS, R"(\+)");
  add_rule(Token::STAR, R"(\*)");
  add_rule(Token::FWD_SLASH, "/");
  add_rule(Token::GE, ">=");
  add_rule(Token::GT, ">");
  add_rule(Token::LE, "<=");
  add_rule(Token::LT, "<");
  add_rule(Token::DBL_EQ, "==");
  add_rule(Token::NE, "!=");
  add_rule(Token::DBL_AND, "&&");
  add_rule(Token::DBL_PIPE, R"(\|\|)");
  add_rule(Token::IF, "if");
  add_rule(Token::ELSE, "else");
  add_rule(Token::RETURN, "return");
  add_rule(Token::PRINT, "print");
  add_rule(Token::EQ, "=");
  add_rule(Token::SEMICOLON, ";");
  add_rule(Token::NONE, "None");
  add_rule(Token::BOOL, "(True|False)");
  add_rule(Token::INT, R"(\d+)");
  add_rule(Token::FLOAT, R"(\d+\.\d*)");
  add_rule(Token::STRING, R"("(\\.|[^"])*")");
  add_rule(Token::ID, R"([[:alpha:]_]\w*)");
  add_rule(Token::UNKNOWN, ".");
  ignore_rule(R"(#[^\n]*)");
  ignore_rule(R"(\s+)");
}

Parser::Parser() : lang::RecursiveDescentParser<Token, Variable>(Variable::PGM) {
  add_rule(Variable::VARLIST,
           {Term::Terminal(Token::ID),
            {Cardinality::Any(), Term::Group({Term::Terminal(Token::COMMA),
                                              Term::Terminal(Token::ID)})}});
  add_rule(
      Variable::EXPLIST,
      {{Cardinality::AtMost(1),
        Term::Group({Term::NonTerminal(Variable::EXPR),
                     {Cardinality::Any(),
                      Term::Group({Term::Terminal(Token::COMMA),
                                   Term::NonTerminal(Variable::EXPR)})}})}});
  add_rule(Variable::TRAILER,
           {Term::Terminal(Token::LPAREN), Term::NonTerminal(Variable::EXPLIST),
            Term::Terminal(Token::RPAREN)});

  add_rule(Variable::LAMBDA_EXPR,
           {Term::Terminal(Token::LAMBDA), Term::NonTerminal(Variable::VARLIST),
            Term::Terminal(Token::COLON), Term::NonTerminal(Variable::EXPR)});

  add_rule(Variable::ATOM,
           {Term::Terminal(Token::LPAREN), Term::NonTerminal(Variable::EXPR),
            Term::Terminal(Token::RPAREN)});
  add_rule(Variable::ATOM, {Term::Terminal(Token::NONE)});
  add_rule(Variable::ATOM, {Term::Terminal(Token::BOOL)});
  add_rule(Variable::ATOM, {Term::Terminal(Token::INT)});
  add_rule(Variable::ATOM, {Term::Terminal(Token::FLOAT)});
  add_rule(Variable::ATOM, {Term::Terminal(Token::STRING)});
  add_rule(Variable::ATOM, {Term::Terminal(Token::ID)});

  add_rule(Variable::ATOM_EXPR,
           {Term::NonTerminal(Variable::ATOM),
            {Cardinality::Any(), Term::NonTerminal(Variable::TRAILER)}});

  add_rule(Variable::MONOP, {Term::Terminal(Token::EXCLAMATION_PT)});
  add_rule(Variable::MONOP, {Term::Terminal(Token::MINUS)});

  add_rule(Variable::BINOP, {Term::Terminal(Token::PLUS)});
  add_rule(Variable::BINOP, {Term::Terminal(Token::MINUS)});
  add_rule(Variable::BINOP, {Term::Terminal(Token::STAR)});
  add_rule(Variable::BINOP, {Term::Terminal(Token::FWD_SLASH)});
  add_rule(Variable::BINOP, {Term::Terminal(Token::GE)});
  add_rule(Variable::BINOP, {Term::Terminal(Token::GT)});
  add_rule(Variable::BINOP, {Term::Terminal(Token::LE)});
  add_rule(Variable::BINOP, {Term::Terminal(Token::LT)});
  add_rule(Variable::BINOP, {Term::Terminal(Token::DBL_EQ)});
  add_rule(Variable::BINOP, {Term::Terminal(Token::NE)});
  add_rule(Variable::BINOP, {Term::Terminal(Token::DBL_AND)});
  add_rule(Variable::BINOP, {Term::Terminal(Token::DBL_PIPE)});

  add_rule(Variable::ARITH_EXPR, {Term::NonTerminal(Variable::MONOP),
                                  Term::NonTerminal(Variable::ATOM_EXPR)});
  add_rule(Variable::ARITH_EXPR, {Term::NonTerminal(Variable::ATOM_EXPR),
                                  Term::NonTerminal(Variable::BINOP),
                                  Term::NonTerminal(Variable::ATOM_EXPR)});
  add_rule(Variable::ARITH_EXPR, {Term::NonTerminal(Variable::ATOM_EXPR)});

  add_rule(Variable::EXPR,
           {Term::Terminal(Token::LAMBDA), Term::NonTerminal(Variable::VARLIST),
            Term::Terminal(Token::COLON), Term::NonTerminal(Variable::EXPR)});
  add_rule(Variable::EXPR,
           {Term::NonTerminal(Variable::ARITH_EXPR), Term::Terminal(Token::IF),
            Term::NonTerminal(Variable::ARITH_EXPR),
            Term::Terminal(Token::ELSE), Term::NonTerminal(ARITH_EXPR)});
  add_rule(Variable::EXPR, {Term::NonTerminal(Variable::ARITH_EXPR)});

  add_rule(Variable::STMT,
           {Term::Terminal(Token::RETURN), Term::NonTerminal(Variable::EXPR),
            Term::Terminal(Token::SEMICOLON)});
  add_rule(Variable::STMT,
           {Term::Terminal(Token::PRINT), Term::NonTerminal(Variable::EXPR),
            Term::Terminal(Token::SEMICOLON)});
  add_rule(Variable::STMT, {Term::NonTerminal(Variable::EXPR),
                            Term::Terminal(Token::SEMICOLON)});
  add_rule(
      Variable::STMT,
      {Term::NonTerminal(Variable::EXPR), Term::Terminal(Token::EQ),
       Term::NonTerminal(Variable::EXPR), Term::Terminal(Token::SEMICOLON)});

  add_rule(Variable::PGM,
           {{Cardinality::AtLeast(1), Term::NonTerminal(Variable::STMT)}});
}

}  // namespace steinlang
