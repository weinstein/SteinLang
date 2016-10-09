#include "lang/cfg_parser.h"

#include <vector> 

#include "lang/parse_tree_util.h"
#include "util/strings.h"

namespace lang {

CfgTokenizer::CfgTokenizer() {
  add_rule(CfgToken::INT, R"(\d+)");
  add_rule(CfgToken::ID, R"([[:alpha:]_]\w*)");
  add_rule(CfgToken::REGEX_TOK, R"("(\\.|[^"])*")");
  add_rule(CfgToken::LITERAL_TOK, R"('(\\.|[^'])*')");
  add_rule(CfgToken::PIPE, R"(\|)");
  add_rule(CfgToken::SEMICOLON, ";");
  add_rule(CfgToken::PRODUCES, "::=");
  add_rule(CfgToken::PRODUCES, ":");
  add_rule(CfgToken::PRODUCES, "=");
  add_rule(CfgToken::PRODUCES, "->");
  add_rule(CfgToken::LPAREN, R"(\()");
  add_rule(CfgToken::RPAREN, R"(\))");
  add_rule(CfgToken::LBRACKET, R"(\[)");
  add_rule(CfgToken::RBRACKET, R"(\])");
  add_rule(CfgToken::LBRACE, R"(\{)");
  add_rule(CfgToken::RBRACE, R"(\})");
  add_rule(CfgToken::COMMA, ",");
  add_rule(CfgToken::STAR, R"(\*)");
  add_rule(CfgToken::PLUS, R"(\+)");
  add_rule(CfgToken::QMARK, R"(\?)");
  add_rule(CfgToken::UNKNOWN, ".");
  // Ignore comments.
  ignore_rule(R"(^#[^\n]*)");
  // Ignore all other whitespace.
  ignore_rule(R"(\s+)");
}

CfgParser::CfgParser()
    : RecursiveDescentParser<CfgToken, CfgSyntax>(CfgSyntax::GRAMMAR) {
  add_rule(CfgSyntax::INT, {Term::Terminal(CfgToken::INT)});
  add_rule(CfgSyntax::ID, {Term::Terminal(CfgToken::ID)});
  add_rule(CfgSyntax::REGEX_TOK, {Term::Terminal(CfgToken::REGEX_TOK)});
  add_rule(CfgSyntax::LITERAL_TOK, {Term::Terminal(CfgToken::LITERAL_TOK)});

  add_rule(CfgSyntax::CARDINALITY, {Term::Terminal(CfgToken::STAR)});
  add_rule(CfgSyntax::CARDINALITY, {Term::Terminal(CfgToken::PLUS)});
  add_rule(CfgSyntax::CARDINALITY, {Term::Terminal(CfgToken::QMARK)});
  add_rule(CfgSyntax::CARDINALITY,
           {Term::Terminal(CfgToken::LBRACE), Term::NonTerminal(CfgSyntax::INT),
            Term::Terminal(CfgToken::RBRACE)});
  add_rule(CfgSyntax::CARDINALITY,
           {Term::Terminal(CfgToken::LBRACE), Term::NonTerminal(CfgSyntax::INT),
            Term::Terminal(CfgToken::COMMA), Term::NonTerminal(CfgSyntax::INT),
            Term::Terminal(CfgToken::RBRACE)});

  add_rule(CfgSyntax::TERM, {Term::Terminal(CfgToken::LPAREN),
                             Term::NonTerminal(CfgSyntax::ALTERNATIVES),
                             Term::Terminal(CfgToken::RPAREN)});
  add_rule(CfgSyntax::TERM, {Term::NonTerminal(CfgSyntax::ID)});
  add_rule(CfgSyntax::TERM, {Term::NonTerminal(CfgSyntax::REGEX_TOK)});
  add_rule(CfgSyntax::TERM, {Term::NonTerminal(CfgSyntax::LITERAL_TOK)});

  add_rule(CfgSyntax::EXPR, {Term::Terminal(CfgToken::LBRACKET),
                             Term::NonTerminal(CfgSyntax::ALTERNATIVES),
                             Term::Terminal(CfgToken::RBRACKET)});
  add_rule(CfgSyntax::EXPR, {Term::NonTerminal(CfgSyntax::TERM),
                             Term::NonTerminal(CfgSyntax::CARDINALITY)});
  add_rule(CfgSyntax::EXPR, {Term::NonTerminal(CfgSyntax::TERM)});

  add_rule(CfgSyntax::EXPRS,
           {{Cardinality::AtLeast(1), Term::NonTerminal(CfgSyntax::EXPR)}});

  add_rule(CfgSyntax::ALTERNATIVES,
           {Term::NonTerminal(CfgSyntax::EXPRS),
            {Cardinality::Any(),
             Term::Group({Term::Terminal(CfgToken::PIPE),
                          Term::NonTerminal(CfgSyntax::EXPRS)})}});

  add_rule(CfgSyntax::RULE, {Term::NonTerminal(CfgSyntax::ID),
                             Term::Terminal(CfgToken::PRODUCES),
                             Term::NonTerminal(CfgSyntax::ALTERNATIVES),
                             Term::Terminal(CfgToken::SEMICOLON)});
  add_rule(CfgSyntax::GRAMMAR,
           {{Cardinality::AtLeast(1), Term::NonTerminal(CfgSyntax::RULE)}});
}

namespace {

template <typename T>
T ToSyntax(const CfgParser::ParseTreeNode& node);

struct Ignore {};
Ignore kIgnore;
template <>
Ignore ToSyntax(const CfgParser::ParseTreeNode& node) {
  return {};
}

template <typename... Ts, std::size_t... Is>
std::tuple<Ts...> ToSyntaxTupleImpl(
    const std::vector<CfgParser::ParseTreeNode>& nodes,
    std::index_sequence<Is...>) {
  return std::make_tuple(
      ToSyntax<typename std::tuple_element<Is, std::tuple<Ts...>>::type>(nodes[Is])...);
}

template <typename... Ts>
bool Match(const std::vector<CfgParser::ParseTreeNode>& nodes,
           const std::vector<util::OneOf<CfgToken, CfgSyntax>>& symbols,
           Ts&... outs) {
  if (Matches<CfgParser>(nodes, symbols)) {
    std::tie(outs...) = ToSyntaxTupleImpl<Ts...>(
        nodes, std::make_index_sequence<sizeof...(Ts)>());
    return true;
  }
  return false;
}

std::string ToId(const CfgParser::ParseTreeNode& node) {
  const CfgParser::ParseTreeNode& child = node.children().front();
  return child.token().value;
}

template <>
std::string ToSyntax(const CfgParser::ParseTreeNode& node) {
  const CfgParser::ParseTreeNode& child = node.children().front();
  return child.token().value;
}

std::string ToRegexStr(const CfgParser::ParseTreeNode& node) {
  const CfgParser::ParseTreeNode& child = node.children().front();
  std::string str = child.token().value;
  str.erase(str.begin());
  str.erase(str.end() - 1);
  util::Unescape(&str);
  return str;
}

std::string ToLiteralStr(const CfgParser::ParseTreeNode& node) {
  const CfgParser::ParseTreeNode& child = node.children().front();
  std::string str = child.token().value;
  str.erase(str.begin());
  str.erase(str.end() - 1);
  util::Unescape(&str);
  util::EscapeForRegex(&str);
  return str;
}

template <>
int ToSyntax(const CfgParser::ParseTreeNode& node) {
  const CfgParser::ParseTreeNode& child = node.children().front();
  return std::stoi(child.token().value);
}

template <>
ParsedGrammar::Parser::Term ToSyntax(const CfgParser::ParseTreeNode& node) {
  if (node.children().size() == 1 && !node.children().front().is_terminal()) {
    const auto& child = node.children().front();
    switch (child.variable_tag()) {
      case CfgSyntax::ID:
        return ParsedGrammar::Parser::Term(
            ParsedGrammar::Parser::Symbol::NonTerminal(ToId(child)));
      case CfgSyntax::REGEX_TOK:
        return ParsedGrammar::Parser::Term(
            ParsedGrammar::Parser::Symbol::Terminal(ToRegexStr(child)));
      case CfgSyntax::LITERAL_TOK:
        return ParsedGrammar::Parser::Term(
            ParsedGrammar::Parser::Symbol::Terminal(ToLiteralStr(child)));
      default:
        break;
    }
  }

  ParsedGrammar::Parser::Alternatives group;
  if (Match(node.children(),
            {CfgToken::LPAREN, CfgSyntax::ALTERNATIVES, CfgToken::RPAREN},
            kIgnore, group, kIgnore)) {
    return ParsedGrammar::Parser::Term(std::move(group));
  }
  return ParsedGrammar::Parser::Term(std::move(group));
}

template <>
ParsedGrammar::Parser::Cardinality ToSyntax(
    const CfgParser::ParseTreeNode& node) {
  const auto& children = node.children();
  if (children.size() == 1 && children.front().is_terminal()) {
    const auto& child = children.front();
    switch (child.token().tag) {
      case CfgToken::STAR:
        return ParsedGrammar::Parser::Cardinality::Any();
      case CfgToken::PLUS:
        return ParsedGrammar::Parser::Cardinality::AtLeast(1);
      case CfgToken::QMARK:
        return ParsedGrammar::Parser::Cardinality::AtMost(1);
      default:
        break;
    }
  }

  int num_reps;
  if (Match(children, {CfgToken::LBRACE, CfgSyntax::INT, CfgToken::RBRACE},
            kIgnore, num_reps, kIgnore)) {
    return ParsedGrammar::Parser::Cardinality::Times(num_reps);
  }

  int min_reps, max_reps;
  if (Match(children, {CfgToken::LBRACE, CfgSyntax::INT, CfgToken::COMMA,
                       CfgSyntax::INT, CfgToken::RBRACE},
            kIgnore, min_reps, kIgnore, max_reps, kIgnore)) {
    return ParsedGrammar::Parser::Cardinality{min_reps, max_reps};
  }

  return ParsedGrammar::Parser::Cardinality::Times(0);
}

template <>
ParsedGrammar::Parser::Expression ToSyntax(
    const CfgParser::ParseTreeNode& node) {
  const auto& children = node.children();

  ParsedGrammar::Parser::Alternatives group;
  if (Match(children,
            {CfgToken::LBRACKET, CfgSyntax::ALTERNATIVES, CfgToken::RBRACKET},
            kIgnore, group, kIgnore)) {
    return ParsedGrammar::Parser::Expression(
        ParsedGrammar::Parser::Cardinality::AtMost(1),
        ParsedGrammar::Parser::Term(std::move(group)));
  }

  auto expr = ParsedGrammar::Parser::Expression::Epsilon();
  if (Match(children, {CfgSyntax::TERM, CfgSyntax::CARDINALITY}, expr.term,
            expr.cardinality)) {
    return expr;
  }
  if (Match(children, {CfgSyntax::TERM}, expr.term)) {
    expr.cardinality = ParsedGrammar::Parser::Cardinality::Times(1);
    return expr;
  }
  return expr;
}

template <>
ParsedGrammar::Parser::Expressions ToSyntax(
    const CfgParser::ParseTreeNode& node) {
  ParsedGrammar::Parser::Expressions exprs;
  for (const auto& child : node.children()) {
    if (!child.is_terminal() && child.variable_tag() == CfgSyntax::EXPR) {
      exprs.push_back(ToSyntax<ParsedGrammar::Parser::Expression>(child));
    }
  }
  return exprs;
}

template <>
ParsedGrammar::Parser::Alternatives ToSyntax(
    const CfgParser::ParseTreeNode& node) {
  ParsedGrammar::Parser::Alternatives alts;
  for (const auto& child : node.children()) {
    if (!child.is_terminal() && child.variable_tag() == CfgSyntax::EXPRS) {
      alts.push_back(ToSyntax<ParsedGrammar::Parser::Expressions>(child));
    }
  }
  return alts;
}

std::vector<std::pair<std::string, ParsedGrammar::Parser::Alternatives>>
ToRules(const CfgParser::ParseTreeNode& node) {
  std::vector<std::pair<std::string, ParsedGrammar::Parser::Alternatives>>
      rules;
  for (const auto& child : node.children()) {
    std::string rule_name;
    ParsedGrammar::Parser::Alternatives alts;
    if (Match(child.children(), {CfgSyntax::ID, CfgToken::PRODUCES,
                                 CfgSyntax::ALTERNATIVES, CfgToken::SEMICOLON},
              rule_name, kIgnore, alts, kIgnore)) {
      rules.push_back({std::move(rule_name), std::move(alts)});
    }
  }
  return rules;
}

void ToTokens(const ParsedGrammar::Parser::Alternatives& alts,
              std::vector<std::string>* out);
void ToTokens(const ParsedGrammar::Parser::Term& term,
              std::vector<std::string>* out) {
  if (term.is_group()) {
    ToTokens(term.group(), out);
  } else if (term.is_terminal()) {
    out->push_back(term.terminal());
  }
}

void ToTokens(const ParsedGrammar::Parser::Alternatives& alts,
              std::vector<std::string>* out) {
  for (const auto& exprs : alts) {
    for (const auto& expr : exprs) {
      ToTokens(expr.term, out);
    }
  }
}

}  // namespace

ParsedGrammar::ParsedGrammar(std::string start_rule,
                             const CfgParser::ParseTreeNode& root)
    : ParsedGrammar(std::move(start_rule)) {
  std::vector<std::pair<std::string, ParsedGrammar::Parser::Alternatives>>
      rules = ToRules(root);
  std::vector<std::string> token_tags;
  for (auto& kv : rules) {
    ToTokens(kv.second, &token_tags);
    const std::string rule_name = std::move(kv.first);
    for (auto& alt : kv.second) {
      parser.add_rule(rule_name, std::move(alt));
    }
  }
  for (const std::string& token_tag : token_tags) {
    tokenizer.add_rule(token_tag, token_tag);
  }
}

}  // namespace lang
