#ifndef LANG_CFGTL_CFG_TYPES_H_
#define LANG_CFGTL_CFG_TYPES_H_

#include <type_traits>

#include "lang/cfgtl/cardinality.h"

namespace lang {

template <typename T, T Tag>
struct Token : std::integral_constant<T, Tag> {};

template <typename T, T Tag>
struct Variable : std::integral_constant<T, Tag> {};

template <typename Term, typename Cardinality>
struct Expression {};

template <typename... Es>
struct ExpressionList {};

template <typename... As>
struct AlternativeList {};

template <typename Lhs, typename Rhs>
struct Rule {};

template <typename... Rs>
struct Grammar {};

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
      std::is_same<typename std::remove_const<Needle>::type,
                   typename std::remove_const<Lhs>::type>::value,
      Rhs, typename Lookup<Grammar<Rs...>>::template TypeOf<Needle>>::type;
};

}  // namespace lang

#endif  // LANG_CFGTL_CFG_TYPES_H_
