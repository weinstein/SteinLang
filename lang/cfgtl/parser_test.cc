#include "lang/cfgtl/parser.h"

#include "lang/tokenizer.h"

namespace lang {

enum class TokTag {
  INT,
  LPAREN,
  RPAREN,
  PIPE,
};

enum class VarTag {
  INT,
  TERM,
  GROUP_TERM,
  ALTERNATIVES,
  EXPRS,
  EXPR,
};

using IntTok = Token<TokTag, TokTag::INT>;
using LParen = Token<TokTag, TokTag::LPAREN>;
using RParen = Token<TokTag, TokTag::RPAREN>;
using Pipe = Token<TokTag, TokTag::PIPE>; 

using IntVar = Variable<VarTag, VarTag::INT>;
using Term = Variable<VarTag, VarTag::TERM>;
using GroupTerm = Variable<VarTag, VarTag::GROUP_TERM>;
using Alternatives = Variable<VarTag, VarTag::ALTERNATIVES>;
using Exprs = Variable<VarTag, VarTag::EXPRS>;
using Expr = Variable<VarTag, VarTag::EXPR>;

using MyLanguage = Grammar<
    Rule<IntVar, IntTok>, Rule<Term, AlternativeList<IntVar, GroupTerm>>,
    Rule<GroupTerm, ExpressionList<LParen, Alternatives, RParen>>,
    Rule<Alternatives,
         ExpressionList<Exprs, Expression<ExpressionList<Pipe, Exprs>, Any>>>,
    Rule<Exprs, Expression<Expr, AtLeast<1>>>,
    Rule<Expr, Term>>;

template <>
struct Parser<MyLanguage, IntVar> {
  using concrete_type = int;

  template <typename It>
  static ParseResult<int, It> Parse(It begin, It end, bool parse_to_end) {
    auto result = DefaultVariableParser<MyLanguage, IntVar>::Parse(
        begin, end, parse_to_end);
    ParseResult<int, It> ret{result.pos, {}};
    if (result.is_success()) {
      ret.data = std::stoi(result.result());
    }
    return ret;
  }
};

using CfgToken = Tokenizer<TokTag>::Token;
using CfgTokenIt = typename std::vector<CfgToken>::iterator;

int Foo(CfgTokenIt begin, CfgTokenIt end) {
  auto result = Parser<MyLanguage, Expr>::Parse(begin, end, true);
  if (result.is_success()) {
    return result.result();
  }
  return -1;
}

}  // namespace lang
