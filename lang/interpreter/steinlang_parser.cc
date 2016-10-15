#include "lang/interpreter/steinlang_parser.h"

#include "lang/interpreter/steinlang_syntax.pb.h"
#include "lang/parse_tree_util.h"
#include "util/one_of.h"
#include "util/strings.h"

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
  add_rule(TokenTag::FLOAT, R"(\d+\.\d*)");
  add_rule(TokenTag::INT, R"(\d+)");
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
  add_rule(VariableTag::TRAILER_LIST,
           {{Cardinality::Any(), Term::NonTerminal(VariableTag::TRAILER)}});

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
            Term::NonTerminal(VariableTag::TRAILER_LIST)});

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

namespace {

template <typename T>
T ToSyntax(const Parser::ParseTreeNode& node);

struct Ignore {};
Ignore kIgnore;
template <>
Ignore ToSyntax(const Parser::ParseTreeNode& node) {
  return {};
}

template <typename... Ts, std::size_t... Is>
std::tuple<Ts...> ToSyntaxTupleImpl(
    const std::vector<Parser::ParseTreeNode>& nodes,
    std::index_sequence<Is...>) {
  return std::make_tuple(
      ToSyntax<typename std::tuple_element<Is, std::tuple<Ts...>>::type>(
          nodes[Is])...);
}

template <typename T>
bool Match(const Parser::ParseTreeNode& node,
           const util::OneOf<TokenTag, VariableTag>& symbol, T& out) {
  if (lang::Matches<Parser>(node, symbol)) {
    out = ToSyntax<T>(node);
    return true;
  }
  return false;
}

template <typename... Ts>
bool Match(const std::vector<Parser::ParseTreeNode>& nodes,
           const std::vector<util::OneOf<TokenTag, VariableTag>>& symbols,
           Ts&... outs) {
  if (lang::Matches<Parser>(nodes, symbols)) {
    std::tie(outs...) = ToSyntaxTupleImpl<Ts...>(
        nodes, std::make_index_sequence<sizeof...(Ts)>());
    return true;
  }
  return false;
}

template <>
Expression ToSyntax(const Parser::ParseTreeNode& node) {
  Expression expr;

  std::vector<Variable> lambda_var_list;
  Expression lambda_body;
  if (Match(node.children(), {TokenTag::LAMBDA, VariableTag::VARLIST,
                              TokenTag::COLON, VariableTag::EXPR},
            kIgnore, lambda_var_list, kIgnore, lambda_body)) {
    *expr.mutable_lambda_exp()->mutable_body() = lambda_body;
    for (const Variable& v : lambda_var_list) {
      *expr.mutable_lambda_exp()->add_param() = v;
    }
    return expr;
  }

  Expression if_exp, cond_exp, else_exp;
  if (Match(node.children(),
            {VariableTag::ARITH_EXPR, TokenTag::IF, VariableTag::ARITH_EXPR,
             TokenTag::ELSE, VariableTag::ARITH_EXPR},
            if_exp, kIgnore, cond_exp, kIgnore, else_exp)) {
    *expr.mutable_tern_exp()->mutable_if_exp() = if_exp;
    *expr.mutable_tern_exp()->mutable_cond_exp() = cond_exp;
    *expr.mutable_tern_exp()->mutable_else_exp() = else_exp;
    return expr;
  }

  MonArithOp mon_op;
  Expression sub_expr;
  if (Match(node.children(), {VariableTag::MONOP, VariableTag::ATOM_EXPR},
            mon_op, sub_expr)) {
    expr.mutable_mon_arith_exp()->set_op(mon_op);
    *expr.mutable_mon_arith_exp()->mutable_exp() = sub_expr;
    return expr;
  }

  BinArithOp bin_op;
  Expression lhs_expr, rhs_expr;
  if (Match(node.children(), {VariableTag::ATOM_EXPR, VariableTag::BINOP,
                              VariableTag::ATOM_EXPR},
            lhs_expr, bin_op, rhs_expr)) {
    expr.mutable_bin_arith_exp()->set_op(bin_op);
    *expr.mutable_bin_arith_exp()->mutable_lhs() = lhs_expr;
    *expr.mutable_bin_arith_exp()->mutable_rhs() = rhs_expr;
    return expr;
  }

  Expression atom;
  std::vector<std::vector<Expression>> trailers;
  if (Match(node.children(), {VariableTag::ATOM, VariableTag::TRAILER_LIST},
            atom, trailers)) {
    for (const std::vector<Expression>& trailer : trailers) {
      FuncAppExpression func_app_expr;
      *func_app_expr.mutable_func() = atom;
      for (const Expression& expr : trailer) {
        *func_app_expr.add_arg() = expr;
      }
      atom.Clear();
      *atom.mutable_func_app_exp() = func_app_expr;
    }
    return atom;
  }

  if (Match(node.children(),
            {TokenTag::LPAREN, VariableTag::EXPR, TokenTag::RPAREN}, kIgnore,
            expr, kIgnore)) {
    return expr;
  }

  if (Match(node.children(), {TokenTag::NONE}, kIgnore)) {
    expr.mutable_lit_exp()->set_none_val(true);
    return expr;
  }

  bool b;
  if (Match(node.children(), {TokenTag::BOOL}, b)) {
    expr.mutable_lit_exp()->set_bool_val(b);
    return expr;
  }

  int64_t i;
  if (Match(node.children(), {TokenTag::INT}, i)) {
    expr.mutable_lit_exp()->set_int_val(i);
    return expr;
  }

  float f;
  if (Match(node.children(), {TokenTag::FLOAT}, f)) {
    expr.mutable_lit_exp()->set_float_val(f);
    return expr;
  }

  std::string s;
  if (Match(node.children(), {TokenTag::STRING}, s)) {
    expr.mutable_lit_exp()->set_str_val(s);
    return expr;
  }

  Variable v;
  if (Match(node.children(), {TokenTag::ID}, v)) {
    *expr.mutable_var_exp() = v;
    return expr;
  }

  if (Match(node.children(), {VariableTag::ATOM_EXPR}, expr)) {
    return expr;
  }
  if (Match(node.children(), {VariableTag::ARITH_EXPR}, expr)) {
    return expr;
  }
  return expr;
}

template <>
std::vector<Variable> ToSyntax(const Parser::ParseTreeNode& node) {
  std::vector<Variable> vars;
  for (const auto& child : node.children()) {
    Variable v;
    if (Match(child, TokenTag::ID, v)) {
      vars.push_back(v);
    }
  }
  return vars;
}

template <>
std::vector<std::vector<Expression>> ToSyntax(
    const Parser::ParseTreeNode& node) {
  std::vector<std::vector<Expression>> trailers;
  for (const auto& child : node.children()) {
    std::vector<Expression> trailer;
    if (Match(child.children(),
              {TokenTag::LPAREN, VariableTag::EXPLIST, TokenTag::RPAREN},
              kIgnore, trailer, kIgnore)) {
      trailers.push_back(trailer);
    }
  }
  return trailers;
}

template <>
std::vector<Expression> ToSyntax(const Parser::ParseTreeNode& node) {
  std::vector<Expression> trailer;
  for (const auto& child : node.children()) {
    Expression expr;
    if (Match(child, VariableTag::EXPR, expr)) {
      trailer.push_back(expr);
    }
  }
  return trailer;
}

template <>
Variable ToSyntax(const Parser::ParseTreeNode& node) {
  Variable v;
  v.set_name(node.token().value);
  return v;
}

template <>
std::string ToSyntax(const Parser::ParseTreeNode& node) {
  std::string s = node.token().value;
  s.erase(s.begin());
  s.erase(s.end() - 1);
  util::Unescape(&s);
  return s;
}

template <>
float ToSyntax(const Parser::ParseTreeNode& node) {
  return std::stof(node.token().value);
}

template <>
int64_t ToSyntax(const Parser::ParseTreeNode& node) {
  return std::stoll(node.token().value);
}

template <>
bool ToSyntax(const Parser::ParseTreeNode& node) {
  return node.token().value == "True";
}

template <>
MonArithOp ToSyntax(const Parser::ParseTreeNode& node) {
  switch (node.children().front().token().tag) {
    case TokenTag::EXCLAMATION_PT:
      return MonArithOp::NOT;
    case TokenTag::MINUS:
      return MonArithOp::NEG;
    default:
      return MonArithOp_MIN;
  }
}

template <>
BinArithOp ToSyntax(const Parser::ParseTreeNode& node) {
  switch (node.children().front().token().tag) {
    case TokenTag::PLUS:
      return BinArithOp::ADD;
    case TokenTag::MINUS:
      return BinArithOp::SUB;
    case TokenTag::STAR:
      return BinArithOp::MUL;
    case TokenTag::FWD_SLASH:
      return BinArithOp::DIV;
    case TokenTag::GE:
      return BinArithOp::GE;
    case TokenTag::GT:
      return BinArithOp::GT;
    case TokenTag::LE:
      return BinArithOp::LE;
    case TokenTag::LT:
      return BinArithOp::LT;
    case TokenTag::DBL_EQ:
      return BinArithOp::EQ;
    case TokenTag::NE:
      return BinArithOp::NE;
    case TokenTag::DBL_AND:
      return BinArithOp::AND;
    case TokenTag::DBL_PIPE:
      return BinArithOp::OR;
    default:
      return BinArithOp_MIN;
  }
}

template <>
Statement ToSyntax(const Parser::ParseTreeNode& node) {
  Statement stmt;
  if (Match(node.children(),
            {TokenTag::RETURN, VariableTag::EXPR, TokenTag::SEMICOLON}, kIgnore,
            *stmt.mutable_ret_stmt(), kIgnore)) {
    return stmt;
  }
  if (Match(node.children(),
            {TokenTag::PRINT, VariableTag::EXPR, TokenTag::SEMICOLON}, kIgnore,
            *stmt.mutable_print_stmt(), kIgnore)) {
    return stmt;
  }
  if (Match(node.children(), {VariableTag::EXPR, TokenTag::SEMICOLON},
            *stmt.mutable_exp_stmt(), kIgnore)) {
    return stmt;
  }
  if (Match(node.children(), {VariableTag::EXPR, TokenTag::EQ,
                              VariableTag::EXPR, TokenTag::SEMICOLON},
            *stmt.mutable_assign_stmt()->mutable_lhs(), kIgnore,
            *stmt.mutable_assign_stmt()->mutable_rhs(), kIgnore)) {
    return stmt;
  }
  return stmt;
}

}  // namespace

Program ToProgram(const Parser::ParseTreeNode& parse_tree) {
  Program pgm;
  for (const auto& child : parse_tree.children()) {
    *pgm.add_stmt() = ToSyntax<Statement>(child);
  }
  return pgm;
}

}  // namespace steinlang
