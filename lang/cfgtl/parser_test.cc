#include "lang/cfgtl/parser.h"

#include "lang/cfgtl/builder.h"
#include "lang/tokenizer.h"

namespace lang {

enum class TokenTag {
  NONE,
  BOOL,
  INT,
  FLOAT,
  STRING,
  ID,
  COMMA,
  LPAREN,
  RPAREN,
  LBRACE,
  RBRACE,
  LBRACKET,
  RBRACKET,
  LAMBDA,
  COLON,
  EXCLAMATION_PT,
  MINUS,
  PLUS,
  STAR,
  FWD_SLASH,
  GE,
  GT,
  LE,
  LT,
  DBL_EQ,
  NE,
  DBL_AND,
  DBL_PIPE,
  IF,
  ELSE,
  RETURN,
  PRINT,
  WHILE,
  FOR,
  DEF,
  EQ,
  SEMICOLON,
  UNKNOWN,
};

template <TokenTag Tag>
using Tok = Token<TokenTag, Tag>;

enum class VariableTag {
  VARLIST,
  EXPLIST,
  TRAILER,
  TRAILER_LIST,
  LAMBDA_EXPR,
  ATOM,
  ATOM_EXPR,
  MONOP,
  BINOP,
  ARITH_EXPR,
  EXPR,
  BLOCK,
  STMT,
  PGM,
};

template <VariableTag Tag>
using Var = Variable<VariableTag, Tag>;

constexpr auto kVarList = Var<VariableTag::VARLIST>();
constexpr auto kId = Tok<TokenTag::ID>();
constexpr auto kComma = Tok<TokenTag::COMMA>();
constexpr auto kExpList = Var<VariableTag::EXPLIST>();
constexpr auto kExpr = Var<VariableTag::EXPR>();

using SteinLang =
    decltype(kVarList |= kId + (kComma + kId) * Any());
//             kExpList |= (kExpr + (kComma + kExpr) * Any()) * Optional());
using MyLanguage = Grammar<
    Rule<Var<VariableTag::VARLIST>,
         ExpressionList<
             Tok<TokenTag::ID>,
             Expression<ExpressionList<Tok<TokenTag::COMMA>, Tok<TokenTag::ID>>,
                        Any>>>>;
static_assert(std::is_same<SteinLang, MyLanguage>::value, "");

template <>
struct Parser<SteinLang, decltype(kVarList)> {
  using concrete_type = std::vector<std::string>;

  template <typename It>
  static ParseResult<concrete_type, It> Parse(It begin, It end,
                                              bool parse_to_end) {
    auto result = DefaultVariableParser<SteinLang, decltype(kVarList)>::Parse(
        begin, end, parse_to_end);
    ParseResult<concrete_type, It> ret{result.pos, {}};
    if (result.is_success()) {
      ret.value().push_back(std::move(std::get<0>(result.value())));
      for (auto& comma_id : std::get<1>(result.value())) {
        ret.value().push_back(std::move(std::get<1>(comma_id)));
      }
    }
    return ret;
  }
};

using SlTokenizer = Tokenizer<TokenTag>;
using SlToken = SlTokenizer::Token;
using SlTokenIt = std::vector<SlToken>::iterator;

std::vector<std::string> ParseVarList(SlTokenIt begin, SlTokenIt end) {
  auto result = Parser<SteinLang, decltype(kVarList)>::Parse(begin, end, true);
  if (result.is_success()) {
    return std::move(result.value());
  }
  return {};
}

}  // namespace lang
