#ifndef LANG_RECURSIVE_DESCENT_PARSER_H_
#define LANG_RECURSIVE_DESCENT_PARSER_H_

#include <sstream>
#include <map>
#include <memory>
#include <vector>

#include "lang/tokenizer.h"
#include "util/one_of.h"

namespace lang {

template <typename TokenTag, typename VariableTag>
class RecursiveDescentParser {
 public:
  typedef TokenTag token_type;
  typedef VariableTag variable_type;

  typedef typename Tokenizer<TokenTag>::Token Token;

  struct Expression;
  typedef std::vector<Expression> Expressions;
  typedef std::vector<Expressions> Alternatives;

  class ParseTreeNode {
   public:
    explicit ParseTreeNode(Token token) : data_(std::move(token)) {}
    explicit ParseTreeNode(VariableTag tag) : data_(std::move(tag)) {}

    ParseTreeNode(const ParseTreeNode&) = delete;
    ParseTreeNode(ParseTreeNode&&) = default;
    ParseTreeNode& operator=(const ParseTreeNode&) = delete;
    ParseTreeNode& operator=(ParseTreeNode&&) = default;

    bool is_terminal() const { return data_.is_first(); }
    const Token& token() const { return data_.first(); }
    const VariableTag& variable_tag() const { return data_.second(); }

    const std::vector<ParseTreeNode>& children() const { return children_; }
    std::vector<ParseTreeNode>& mutable_children() { return children_; }

    std::string DebugString() const { return DebugString(0); }
    std::string DebugString(int indent) const;

   private:
    util::OneOf<Token, VariableTag> data_;
    std::vector<ParseTreeNode> children_;
  };

  template <typename TokenIt>
  struct ParseResult {
    std::string DebugString(TokenIt end) const;

    TokenIt pos;
    ParseTreeNode node;
    bool success;
  };

  template <typename TokenIt>
  struct ParseResults {
    ParseResults(ParseResult<TokenIt> result)
        : pos(result.pos), success(result.success) {
      nodes.push_back(std::move(result.node));
    }

    ParseResults(TokenIt pos, std::vector<ParseTreeNode> nodes, bool success)
        : pos(pos), nodes(std::move(nodes)), success(success) {}

    TokenIt pos;
    std::vector<ParseTreeNode> nodes;
    bool success;
  };

  class Symbol {
   public:
    Symbol(util::OneOf<TokenTag, VariableTag> data)
      : data_(std::move(data)) {}

    static Symbol Terminal(TokenTag tag) {
      return Symbol({std::move(tag), util::OfFirst()});
    }

    static Symbol NonTerminal(VariableTag tag) {
      return Symbol({std::move(tag), util::OfSecond()});
    }

    bool is_terminal() const { return data_.is_first(); }
    const TokenTag& terminal() const { return data_.first(); }
    const VariableTag& nonterminal() const { return data_.second(); }

   private:
    util::OneOf<TokenTag, VariableTag> data_;
  };

  class Term {
   public:
    explicit Term(Symbol s) : data_(std::move(s)) {}

    explicit Term(Alternatives alts) : data_(std::move(alts)) {}

    static Term Terminal(TokenTag tag) {
      return Term{Symbol::Terminal(std::move(tag))};
    }

    static Term NonTerminal(VariableTag tag) {
      return Term{Symbol::NonTerminal(std::move(tag))};
    }

    static Term Group(Expressions exprs) { return Term{{std::move(exprs)}}; }
    static Term Group(Alternatives alts) { return Term{std::move(alts)}; }

    bool is_symbol() const { return data_.is_first(); }
    const Symbol& symbol() const { return data_.first(); }

    bool is_terminal() const { return is_symbol() && symbol().is_terminal(); }
    const TokenTag& terminal() const { return symbol().terminal(); }

    bool is_nonterminal() const {
      return is_symbol() && !symbol().is_terminal();
    }
    const VariableTag& nonterminal() const { return symbol().nonterminal(); }

    bool is_group() const { return data_.is_second(); }
    const Alternatives& group() const { return data_.second(); }

   private:
    util::OneOf<Symbol, Alternatives> data_;
  };

  struct Cardinality {
    static Cardinality Times(int n) { return {n, n}; }
    static Cardinality AtLeast(int n) {
      return {n, std::numeric_limits<int>::max()};
    }
    static Cardinality AtMost(int n) { return {0, n}; }
    static Cardinality Any() { return AtLeast(0); }

    int min = 1;
    int max = 1;
  };

  struct Expression {
    Expression(Term term) : term(std::move(term)) {}
    Expression(Cardinality cardinality, Term term)
        : cardinality(std::move(cardinality)), term(std::move(term)) {}

    static Expression Epsilon() {
      return Expression{Cardinality::Times(0), Term({})};
    }

    Cardinality cardinality;
    Term term;
  };

  explicit RecursiveDescentParser(VariableTag root) : root_(std::move(root)) {}

  void add_rule(VariableTag lhs, Expressions rhs) {
    rules_[lhs].push_back(std::move(rhs));
  }

  const std::map<VariableTag, Alternatives>& rules() const { return rules_; }

  template <typename TokenIt>
  ParseResult<TokenIt> Parse(TokenIt begin, TokenIt end) const {
    return ParseNonTerminal(begin, end, root_, true);
  }

 private:
  template <typename TokenIt>
  ParseResult<TokenIt> ParseTerminal(TokenIt begin, TokenIt end,
                                     const TokenTag& tag,
                                     bool parse_to_end) const;

  template <typename TokenIt>
  ParseResult<TokenIt> ParseNonTerminal(TokenIt begin, TokenIt end,
                                        const VariableTag& tag,
                                        bool parse_to_end) const;

  template <typename TokenIt>
  ParseResults<TokenIt> ParseAlternatives(TokenIt begin, TokenIt end,
                                         const Alternatives& alts,
                                         bool parse_to_end) const;

  template <typename TokenIt>
  ParseResults<TokenIt> ParseExpressions(TokenIt begin, TokenIt end,
                                        const Expressions& exprs,
                                        bool parse_to_end) const;

  template <typename TokenIt>
  ParseResults<TokenIt> ParseExpression(TokenIt begin, TokenIt end,
                                       const Expression& expr,
                                       bool parse_to_end) const;

  template <typename TokenIt>
  ParseResults<TokenIt> ParseOptionalTerm(TokenIt begin, TokenIt end,
                                         const Term& term,
                                         bool parse_to_end) const;

  template <typename TokenIt>
  ParseResults<TokenIt> ParseTerm(TokenIt begin, TokenIt end,
                                  const Term& term,
                                  bool parse_to_end) const;

  const VariableTag root_;
  std::map<VariableTag, Alternatives> rules_;
};

template <typename TokenTag, typename VariableTag>
template <typename TokenIt>
RecursiveDescentParser<TokenTag, VariableTag>::ParseResult<TokenIt>
RecursiveDescentParser<TokenTag, VariableTag>::ParseTerminal(
    TokenIt begin, TokenIt end, const TokenTag& tag, bool parse_to_end) const {
  if (begin < end && begin->tag == tag) {
    return ParseResult<TokenIt>{begin + 1, ParseTreeNode(*begin),
                                (!parse_to_end || begin + 1 == end)};
  } else {
    return ParseResult<TokenIt>{begin, ParseTreeNode(Token{tag, "", -1}),
                                false};
  }
}

template <typename TokenTag, typename VariableTag>
template <typename TokenIt>
RecursiveDescentParser<TokenTag, VariableTag>::ParseResult<TokenIt>
RecursiveDescentParser<TokenTag, VariableTag>::ParseNonTerminal(
    TokenIt begin, TokenIt end, const VariableTag& tag,
    bool parse_to_end) const {
  auto it = rules_.find(tag);
  if (it == rules_.end()) {
    return ParseResult<TokenIt>{begin, ParseTreeNode(tag), false};
  }
  const Alternatives& alts = it->second;

  auto results = ParseAlternatives(begin, end, alts, parse_to_end);
  ParseTreeNode node(tag);
  for (auto& result : results.nodes) {
    node.mutable_children().push_back(std::move(result));
  }
  return ParseResult<TokenIt>{results.pos, std::move(node), results.success};
}

template <typename TokenTag, typename VariableTag>
template <typename TokenIt>
RecursiveDescentParser<TokenTag, VariableTag>::ParseResults<TokenIt>
RecursiveDescentParser<TokenTag, VariableTag>::ParseAlternatives(
    TokenIt begin, TokenIt end, const Alternatives& alts,
    bool parse_to_end) const {
  ParseResults<TokenIt> error_results{begin, {}, false};
  for (const Expressions& exprs : alts) {
    auto exprs_results = ParseExpressions(begin, end, exprs, parse_to_end);
    if (exprs_results.success) {
      return std::move(exprs_results);
    } else if (exprs_results.pos >= error_results.pos) {
      error_results = std::move(exprs_results);
    }
  }
  return error_results;
}

template <typename TokenTag, typename VariableTag>
template <typename TokenIt>
RecursiveDescentParser<TokenTag, VariableTag>::ParseResults<TokenIt>
RecursiveDescentParser<TokenTag, VariableTag>::ParseExpressions(
    TokenIt begin, TokenIt end,
    const Expressions& exprs, bool parse_to_end) const {
  ParseResults<TokenIt> cur{begin, {}, true};
  for (auto it = exprs.begin(); it != exprs.end(); ++it) {
    auto expr_results = ParseExpression(
        cur.pos, end, *it, (parse_to_end && it + 1 == exprs.end()));
    cur.success &= expr_results.success;
    cur.pos = expr_results.pos;
    for (auto& child : expr_results.nodes) {
      cur.nodes.push_back(std::move(child));
    }
    if (!cur.success) {
      break;
    }
  }
  return cur;
}

template <typename TokenTag, typename VariableTag>
template <typename TokenIt>
RecursiveDescentParser<TokenTag, VariableTag>::ParseResults<TokenIt>
RecursiveDescentParser<TokenTag, VariableTag>::ParseExpression(
    TokenIt begin, TokenIt end, const Expression& expr,
    bool parse_to_end) const {
  ParseResults<TokenIt> cur{begin, {}, true};
  for (int i = 0; i < expr.cardinality.max; ++i) {
    const bool is_optional = i >= expr.cardinality.min && cur.success;
    const bool is_end = parse_to_end && (i + 1 == expr.cardinality.max);
    auto results = is_optional
                       ? ParseOptionalTerm(cur.pos, end, expr.term, is_end)
                       : ParseTerm(cur.pos, end, expr.term, is_end);
    cur.success &= results.success;
    auto prev_pos = cur.pos;
    cur.pos = results.pos;
    for (auto& child : results.nodes) {
      cur.nodes.push_back(std::move(child));
    }
    if (cur.pos <= prev_pos) {
      break;
    }
  }
  if (parse_to_end && cur.pos < end) {
    cur.success = false;
  }
  return cur;
}

template <typename TokenTag, typename VariableTag>
template <typename TokenIt>
RecursiveDescentParser<TokenTag, VariableTag>::ParseResults<TokenIt>
RecursiveDescentParser<TokenTag, VariableTag>::ParseOptionalTerm(
    TokenIt begin, TokenIt end, const Term& term,
    bool parse_to_end) const {
  auto results = ParseTerm(begin, end, term, parse_to_end);
  if (results.success) {
    return std::move(results);
  }
  return ParseResults<TokenIt>{begin, {}, (!parse_to_end || begin == end)};
}

template <typename TokenTag, typename VariableTag>
template <typename TokenIt>
RecursiveDescentParser<TokenTag, VariableTag>::ParseResults<TokenIt>
RecursiveDescentParser<TokenTag, VariableTag>::ParseTerm(
    TokenIt begin, TokenIt end, const Term& term, bool parse_to_end) const {
  if (term.is_terminal()) {
    return ParseTerminal(begin, end, term.terminal(), parse_to_end);
  } else if (term.is_nonterminal()) {
    return ParseNonTerminal(begin, end, term.nonterminal(), parse_to_end);
  } else {
    return ParseAlternatives(begin, end, term.group(), parse_to_end);
  }
}

template <typename TokenTag, typename VariableTag>
std::string RecursiveDescentParser<
    TokenTag, VariableTag>::ParseTreeNode::DebugString(int indent) const {
  std::stringstream ss;
  if (is_terminal()) {
    ss << std::string(indent, ' ') << "token: (" << token().tag << ") "
       << token().value << "\n";
  } else {
    ss << std::string(indent, ' ') << "variable: " << variable_tag() << "\n";
  }
  for (const auto& child : children()) {
    ss << child.DebugString(indent + 2);
  }
  return ss.str();
}

template <typename TokenTag, typename VariableTag>
template <typename TokenIt>
std::string RecursiveDescentParser<
    TokenTag, VariableTag>::ParseResult<TokenIt>::DebugString(TokenIt end) const {
  std::stringstream ss;
  ss << "ParseResult {\n"
     << "  success: " << success << "\n"
     << "  tokens: {\n";
  for (auto it = pos; it != end; ++it) {
    ss << "    (" << it->tag << ") " << it->value << "\n";
  }
  ss << "  }\n"
     << "  node: {\n"
     << node.DebugString(4) << "  }\n"
     << "}";
  return ss.str();
}

}  // namespace lang

#endif  // LANG_RECURSIVE_DESCENT_PARSER_H_
