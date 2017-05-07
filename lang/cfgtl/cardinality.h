#ifndef LANG_CFGTL_CARDINALITY_H_
#define LANG_CFGTL_CARDINALITY_H_

#include <cstddef>
#include <limits>

namespace lang {

class CardinalityInterface {};

template <std::size_t Min, std::size_t Max>
struct Cardinality : public CardinalityInterface {
  static constexpr std::size_t min = Min;
  static constexpr std::size_t max = Max;

  static const bool Matches(std::size_t N) { return N >= Min && N <= Max; }
};

template <std::size_t N>
using Times = Cardinality<N, N>;

template <std::size_t N>
using AtLeast = Cardinality<N, std::numeric_limits<std::size_t>::max()>;

template <std::size_t N>
using AtMost = Cardinality<0, N>;

using Any = AtLeast<0>;

using Optional = AtMost<1>;

}  // namespace lang

#endif  // LANG_CFGTL_CARDINALITY_H_
