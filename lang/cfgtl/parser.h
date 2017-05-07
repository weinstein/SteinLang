#ifndef LANG_CFGTL_PARSER_H_
#define LANG_CFGTL_PARSER_H_

#include <array>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "lang/cfgtl/cardinality.h"
#include "lang/cfgtl/cfg_types.h"
#include "util/optional.h"

namespace lang {

template <typename T, typename It>
struct ParseResult {
  bool is_success() const { return data.is_present(); }

  const T& result() const { return data.value(); }
  T& result() { return data.mutable_value(); }

  It pos;
  util::Optional<T> data;
};

template <typename Grammar, typename T>
struct Parser;

template <typename Grammar, typename T, T Tag>
struct Parser<Grammar, Token<T, Tag>> {
  using concrete_type = std::string;

  template <typename It>
  using Result = ParseResult<concrete_type, It>;

  template <typename It>
  static Result<It> Parse(It begin, It end, bool parse_to_end) {
    Result<It> result{begin, {}};
    if (begin < end && begin->tag == Tag) {
      result.pos = begin + 1;
      if (!parse_to_end || result.pos == end) {
        result.data = begin->value;
      }
    }
  };
};

// Specialization of Parser for an Expression of exactly 1 Term.
// This is just the Parser for the Term.
template <typename Grammar, typename Term>
struct Parser<Grammar, Expression<Term, Times<1>>> : Parser<Grammar, Term> {};

// Specialization of Parser for an Expression of an optional Term.
// This will parse a util::Optional.
template <typename Grammar, typename Term>
struct Parser<Grammar, Expression<Term, Optional>> {
  using concrete_type =
      util::Optional<typename Parser<Grammar, Term>::concrete_type>;

  template <typename It>
  using Result = ParseResult<concrete_type, It>;

  template <typename It>
  static Result<It> Parse(It begin, It end, bool parse_to_end) {
    // If we can parse the term, that's the result.
    auto term_result = Parser<Grammar, Term>::Parse(begin, end, parse_to_end);
    if (term_result.success()) {
      // Confusing: the result has data present, and the present value is an
      // optional with the parsed term present.
      concrete_type concrete_result = std::move(term_result.result());
      return Result<It>{term_result.pos, std::move(concrete_result)};
    }
    // We couldn't parse the term.
    // Confusing: the result has data present, and the present value is an empty
    // optional (parse is successful, and we didn't parse the optional term).
    concrete_type concrete_result = {};
    return Result<It>{begin, std::move(concrete_result)};
  }
};

// Specialization of Parser for an Expression of exactly N Terms.
// This will parse a std::array.
template <typename Grammar, typename Term, std::size_t N>
struct Parser<Grammar, Expression<Term, Times<N>>> {
  using concrete_type =
      std::array<typename Parser<Grammar, Term>::concrete_type, N>;

  template <typename It>
  using Result = ParseResult<concrete_type, It>;

  template <typename It>
  static Result<It> Parse(It begin, It end, bool parse_to_end) {
    Result<It> cur{begin, concrete_type()};
    for (std::size_t i = 0; i < N; ++i) {
      const bool is_end = i + 1 == N;
      auto result =
          Parser<Grammar, Term>::Parse(cur.pos, end, is_end && parse_to_end);
      if (!result.is_success()) {
        cur.data.clear();
        return cur;
      }
      cur.pos = result.pos;
      cur.result()[i] = std::move(result.result());
    }
    if (parse_to_end && cur.pos < end) {
      cur.data.clear();
    }
    return cur;
  }
};

// General case for parsing an Expression of a variable number of Terms.
// This will parse a std::vector.
template <typename Grammar, typename Term, typename Cardinality>
struct Parser<Grammar, Expression<Term, Cardinality>> {
  using concrete_type =
      std::vector<typename Parser<Grammar, Term>::concrete_type>;

  template <typename It>
  using Result = ParseResult<concrete_type, It>;

  template <typename It>
  static Result<It> Parse(It begin, It end, bool parse_to_end) {
    Result<It> cur{begin, concrete_type()};
    for (std::size_t i = 0; i < Cardinality::max; ++i) {
      const bool is_end = i + 1 == Cardinality::max;
      const bool is_optional = i >= Cardinality::min;
      if (is_optional) {
        // This can never fail, so result.is_success() is always true and
        // result.result() is always valid.
        auto result = Parser<Grammar, Expression<Term, Optional>>::Parse(
            cur.pos, end, is_end && parse_to_end);
        auto term_or = result.result();
        // If we successfully parsed a term, add it to the results and update
        // the current iterator position.
        if (term_or.is_present()) {
          cur.result().push_back(std::move(term_or.mutable_value()));
          cur.pos = result.pos;
        } else {
          // If we didn't parse a term, we won't be updating the iterator
          // position, so we would never parse the term in the next iteration
          // either. We're done.
          break;
        }
      } else {
        auto result = Parser<Grammar, Term>::Parse(cur.pos, end, is_end && parse_to_end);
        if (!result.is_success()) {
          cur.data.clear();
          return cur;
        }
        cur.pos = result.pos;
        cur.result().push_back(std::move(result.result()));
      }
    }
    if (parse_to_end && cur.pos < end) {
      cur.data.clear();
    }
    return cur;
  }
};

// Specialization of Parser for expression lists containing only one element.
// This will parse the expression directly.
template <typename Grammar, typename E>
struct Parser<Grammar, ExpressionList<E>> : Parser<Grammar, E> {};

template <typename... Ts>
bool And(const Ts&... ts);

template <>
bool And() {
  return true;
}

template <typename T1, typename... Ts>
bool And(const T1& t1, const Ts&... ts) {
  return t1 && And(ts...);
}

// General case for parsing a list of multiple 
template <typename Grammar, typename... Es>
struct Parser<Grammar, ExpressionList<Es...>> {
  using concrete_type =
      std::tuple<typename Parser<Grammar, Es>::concrete_type...>;

  static constexpr std::size_t size = sizeof...(Es);

  template <typename It>
  using Result = ParseResult<concrete_type, It>;

  template <typename It>
  static Result<It> Parse(It begin, It end, bool parse_to_end) {
    return ParseImpl(begin, end, parse_to_end,
                     std::make_index_sequence<size>());
  }

 private:
  template <std::size_t I>
  using IthParser =
      Parser<Grammar, typename std::tuple_element<I, std::tuple<Es...>>::type>;

  template <typename It, std::size_t I>
  static typename IthParser<I>::template Result<It> ParseImpl(
      It* cur_pos, It end, bool parse_to_end) {
    auto result = IthParser<I>::Parse(*cur_pos, end, parse_to_end);
    *cur_pos = result.pos;
    return std::move(result);
  }

  template <typename It, std::size_t... Is>
  static Result<It> ParseImpl(It begin, It end, bool parse_to_end,
                              std::index_sequence<Is...>) {
    It cur_pos = begin;
    std::tuple<typename Parser<Grammar, Es>::template Result<It>...> results =
        std::make_tuple(
            ParseImpl(&cur_pos, end, Is == size - 1 && parse_to_end)...);
    if (And(std::get<Is>(results).is_success()...)) {
      return Result<It>{
          cur_pos,
          std::make_tuple(std::move(std::get<Is>(results).result())...)};
    }
    return Result<It>{cur_pos, {}};
  }
};

// Specialization of Parser for alternative lists of length 1.
// This parses the alternative directly.
template <typename Grammar, typename A>
struct Parser<Grammar, AlternativeList<A>> : Parser<Grammar, A> {};

// Default variable parser. This is just a parser for the variable's rule in the
// grammar.
// This is defined independently of the variable Parser<> to allow further
// variable specialization to access the default variable parser.
template <typename Grammar, typename V>
struct DefaultVariableParser
    : Parser<Grammar, typename Lookup<Grammar>::template TypeOf<V>> {};

template <typename Grammar, typename T, T Tag>
struct Parser<Grammar, Variable<T, Tag>>
    : DefaultVariableParser<Grammar, Variable<T, Tag>> {};

}  // namespace lang 

#endif  // LANG_CFGTL_PARSER_H_
