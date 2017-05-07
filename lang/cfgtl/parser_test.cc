#include "lang/cfgtl/parser.h"

namespace lang {

enum class TokTag {
  INT,
};

enum class VarTag {
  INT,
  FOO,
  BAR,
};

using IntTok = Token<TokTag, TokTag::INT>;
using IntVar = Variable<VarTag, VarTag::INT>;
using Foo = Variable<VarTag, VarTag::FOO>;
using Bar = Variable<VarTag, VarTag::BAR>;
using MyLanguage = Grammar<
    Rule<IntVar, AlternativeList<ExpressionList<Expression<IntTok, Times<1>>>>>,
    Rule<Foo, AlternativeList<ExpressionList<Expression<IntVar, Optional>>>>,
    Rule<Bar, AlternativeList<ExpressionList<Expression<Foo, Times<1>>>>>>;

template <>
struct Parser<MyLanguage, IntVar> {
  using concrete_type = int;

  template <typename It>
  using Result = ParseResult<int, It>;

  template <typename It>
  static Result<It> Parse(It begin, It end, bool parse_to_end) {
    auto result = DefaultVariableParser<MyLanguage, IntVar>::Parse(
        begin, end, parse_to_end);
    Result<It> ret{result.pos, {}};
    if (result.is_success()) {
      ret.data = std::stoi(result.result());
    }
    return ret;
  }
};

template <>
struct Parser<MyLanguage, Bar> {
  using concrete_type = std::string;

  template <typename It>
  using Result = ParseResult<std::string, It>;

  template <typename It>
  static Result<It> Parse(It begin, It end, bool parse_to_end) {
    auto result =
        DefaultVariableParser<MyLanguage, Bar>::Parse(begin, end, parse_to_end);
    Result<It> ret{result.pos, {}};
    if (result.is_success()) {
      ret.data = result.result().is_present()
                     ? std::to_string(result.result().value())
                     : "<none>";
    }
    return ret;
  }
};

}  // namespace lang
