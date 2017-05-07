#ifndef LANG_CFGTL_CFG_TYPES_H_
#define LANG_CFGTL_CFG_TYPES_H_

#include <type_traits>

#include "lang/cfgtl/cardinality.h"

namespace lang {

class TermInterface {};

template <typename T, T Tag>
struct Token : std::integral_constant<T, Tag>, public TermInterface {};

class VariableInterface {};

template <typename T, T Tag>
struct Variable : std::integral_constant<T, Tag>,
                  public TermInterface,
                  public VariableInterface {};

class ExpressionInterface {};

template <typename Term, typename Cardinality>
struct Expression : public ExpressionInterface {
  using term = Term;
  using cardinality = Cardinality;

  static_assert(
      std::is_base_of<TermInterface, Term>::value,
      "Expression contains invalid Term type!");
  static_assert(
      std::is_base_of<CardinalityInterface, Cardinality>::value,
      "Expression contains invalid Cardinality type!");
};

// TODO: replace with std::conjunction.
template <typename...>
struct conjunction : std::true_type {};
template <typename B1>
struct conjunction<B1> : B1 {};
template <typename B1, typename... Bs>
struct conjunction<B1, Bs...>
    : std::conditional<bool(B1::value), conjunction<Bs...>, B1> {};

class ExpressionListInterface {};

template <typename... Es>
struct ExpressionList : public ExpressionListInterface, public TermInterface {
  static_assert(conjunction<std::is_base_of<ExpressionInterface, Es>...>::value,
                "ExpressionList contains invalid Expression types!");
};

class AlternativeListInterface {};

template <typename... As>
struct AlternativeList : public AlternativeListInterface {
  static_assert(
      conjunction<std::is_base_of<ExpressionListInterface, As>...>::value,
      "AlternativeList contains invalid ExpressionList types!");
};

class RuleInterface {};

template <typename Lhs, typename Rhs>
struct Rule : public RuleInterface {
  using lhs = Lhs;
  using rhs = Rhs;

  static_assert(std::is_base_of<VariableInterface, Lhs>::value,
                "Lhs is not a Variable!");
  static_assert(std::is_base_of<AlternativeListInterface, Rhs>::value,
                "Rhs is not an AlternativeList!");
};

template <typename... Rs>
struct Grammar {
  static_assert(conjunction<std::is_base_of<RuleInterface, Rs>...>::value,
                "Grammar contains invalid Rule types!");
};

template <typename Grammar>
struct Lookup;

template <>
struct Lookup<Grammar<>> {
  template <typename Needle>
  using TypeOf = void;
};

template <typename Lhs, typename Rhs, typename... Rs>
struct Lookup<Grammar<Rule<Lhs, Rhs>, Rs...>> {
  template <typename Needle>
  using TypeOf = typename std::conditional<
      std::is_same<Needle, Lhs>::value, Rhs,
      typename Lookup<Grammar<Rs...>>::template TypeOf<Needle>>::type;
};

}  // namespace lang

#endif  // LANG_CFGTL_CFG_TYPES_H_
