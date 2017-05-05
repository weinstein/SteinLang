#ifndef LANG_PARSE_TREE_UTIL_H_
#define LANG_PARSE_TREE_UTIL_H_

#include <vector>
#include <utility>

#include "lang/recursive_descent_parser.h"

namespace lang {

template <typename Parser>
bool Matches(const typename Parser::ParseTreeNode& node,
             const typename Parser::Symbol& s) {
  if (node.is_terminal()) {
    return s.is_terminal() && node.token().tag == s.terminal();
  } else {
    return !s.is_terminal() && node.variable_tag() == s.nonterminal();
  }
}

template <typename Parser>
bool Matches(const typename Parser::ParseTreeNode& node,
             const util::OneOf<typename Parser::token_type,
                               typename Parser::variable_type>& s) {
  if (node.is_terminal()) {
    return s.is_first() && node.token().tag == s.first();
  } else {
    return s.is_second() && node.variable_tag() == s.second();
  }
}

template <typename Parser>
bool Matches(const std::vector<typename Parser::ParseTreeNode>& nodes,
             const std::vector<typename Parser::Symbol>& symbols) {
  if (nodes.size() != symbols.size()) {
    return false;
  }
  for (auto i = 0; i < symbols.size(); ++i) {
    if (!Matches(nodes[i], symbols[i])) {
      return false;
    }
  }
  return true;
}

template <typename Parser>
bool Matches(
    const std::vector<typename Parser::ParseTreeNode>& nodes,
    const std::vector<util::OneOf<typename Parser::token_type,
                                  typename Parser::variable_type>>& symbols) {
  if (nodes.size() != symbols.size()) {
    return false;
  }
  for (auto i = 0; i < symbols.size(); ++i) {
    if (!Matches<Parser>(nodes[i], symbols[i])) {
      return false;
    }
  }
  return true;
}

}  // namespace lang

#endif  // LANG_PARSE_TREE_UTIL_H_
