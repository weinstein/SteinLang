#ifndef LANG_CFGTL_BUILDER_H_
#define LANG_CFGTL_BUILDER_H_

#include "lang/cfgtl/cfg_types.h"

namespace lang {

//template <typename Lhs, typename T, T Tag>
//ExpressionList<Lhs, Token<T, Tag>> operator+(const Lhs&, const Token<T, Tag>&);

//template <typename Lhs, typename T, T Tag>
//ExpressionList<Lhs, Variable<T, Tag>> operator+(const Lhs&,
//                                                const Variable<T, Tag>&);

//template <typename Lhs, typename... Es>
//ExpressionList<Lhs, Es...> operator+(const Lhs&, const ExpressionList<Es...>&);

template <typename T, T Tag, typename Rhs>
ExpressionList<Token<T, Tag>, Rhs> operator+(const Token<T, Tag>&, const Rhs&);

template <typename T, T Tag, typename Rhs>
ExpressionList<Variable<T, Tag>, Rhs> operator+(const Variable<T, Tag>&,
                                                const Rhs&);

template <typename Rhs, typename... Es>
ExpressionList<Es..., Rhs> operator+(const ExpressionList<Es...>&, const Rhs&);

template <typename T, std::size_t N, std::size_t M>
Expression<T, Cardinality<N, M>> operator*(const T&, const Cardinality<N, M>&);

template <typename T, T Tag, typename Rhs>
Grammar<Rule<Variable<T, Tag>, Rhs>> operator|=(const Variable<T, Tag>&, const Rhs&);

template <typename T, typename... Rules>
Grammar<Rules..., T> operator,(const Grammar<Rules...>&, const Grammar<T>&);

template <typename T1, typename T2>
AlternativeList<T1, T2> operator||(const T1&, const T2&);

}  // namespace lang

#endif  // LANG_CFGTL_BUILDER_H_
