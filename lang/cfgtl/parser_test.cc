#include "lang/cfgtl/parser.h"

#include "lang/cfgtl/builder.h"
#include "lang/tokenizer.h"
#include "util/variant.h"

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

auto kNone = Tok<TokenTag::NONE>();
auto kBool = Tok<TokenTag::BOOL>();
auto kInt = Tok<TokenTag::INT>();
auto kFloat = Tok<TokenTag::FLOAT>();
auto kString = Tok<TokenTag::STRING>();
auto kId = Tok<TokenTag::ID>();
auto kComma = Tok<TokenTag::COMMA>();
auto kLParen = Tok<TokenTag::LPAREN>();
auto kRParen = Tok<TokenTag::RPAREN>();
auto kLBrace = Tok<TokenTag::LBRACE>();
auto kRBrace = Tok<TokenTag::RBRACE>();
auto kLBracket = Tok<TokenTag::LBRACKET>();
auto kRBracket = Tok<TokenTag::RBRACKET>();
auto kLambda = Tok<TokenTag::LAMBDA>();
auto kColon = Tok<TokenTag::COLON>();
auto kExclamationPt = Tok<TokenTag::EXCLAMATION_PT>();
auto kMinus = Tok<TokenTag::MINUS>();
auto kPlus = Tok<TokenTag::PLUS>();
auto kStar = Tok<TokenTag::STAR>();
auto kForwardSlash = Tok<TokenTag::FWD_SLASH>();
auto kGe = Tok<TokenTag::GE>();
auto kGt = Tok<TokenTag::GT>();
auto kLe = Tok<TokenTag::LE>();
auto kLt = Tok<TokenTag::LT>();
auto kDblEq = Tok<TokenTag::DBL_EQ>();
auto kNe = Tok<TokenTag::NE>();
auto kDblAnd = Tok<TokenTag::DBL_AND>();
auto kDblPipe = Tok<TokenTag::DBL_PIPE>();
auto kIf = Tok<TokenTag::IF>();
auto kElse = Tok<TokenTag::ELSE>();
auto kReturn = Tok<TokenTag::RETURN>();
auto kPrint = Tok<TokenTag::PRINT>();
auto kWhile = Tok<TokenTag::WHILE>();
auto kFor = Tok<TokenTag::FOR>();
auto kDef = Tok<TokenTag::DEF>();
auto kEq = Tok<TokenTag::EQ>();
auto kSemicolon = Tok<TokenTag::SEMICOLON>();

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

auto kVarList = Var<VariableTag::VARLIST>();
auto kExpList = Var<VariableTag::EXPLIST>();
auto kTrailer = Var<VariableTag::TRAILER>();
auto kTrailerList = Var<VariableTag::TRAILER_LIST>();
auto kLambdaExpr = Var<VariableTag::LAMBDA_EXPR>();
auto kAtom = Var<VariableTag::ATOM>();
auto kAtomExpr = Var<VariableTag::ATOM_EXPR>();
auto kMonOp = Var<VariableTag::MONOP>();
auto kBinOp = Var<VariableTag::BINOP>();
auto kArithExpr = Var<VariableTag::ARITH_EXPR>();
auto kExpr = Var<VariableTag::EXPR>();
auto kBlock = Var<VariableTag::BLOCK>();
auto kStmt = Var<VariableTag::STMT>();
auto kPgm = Var<VariableTag::PGM>();

using SteinLang = decltype(
    kVarList |= kId + (kComma + kId) * Any(),
    kExpList |= (kExpr + (kComma + kExpr) * Any()) * Optional(),
    kTrailer |= kLParen + kExpList + kRParen,
    kTrailerList |= kTrailer * Any(),

    kAtom |= kLParen + kExpr + kRParen || kNone || kBool || kInt || kFloat ||
             kString || kId,
    kAtomExpr |= kAtom + kTrailerList,
    
    kMonOp |= kExclamationPt || kMinus,
    kBinOp |= kPlus || kMinus || kStar || kForwardSlash || kGe || kGt || kLe ||
              kLt || kDblEq || kNe || kDblAnd || kDblPipe,
    kArithExpr |=
    kMonOp + kAtomExpr || kAtomExpr + kBinOp + kAtomExpr || kAtomExpr,
    kExpr |= kLambda + kVarList + kColon + kExpr ||
             kLambda + kVarList + kBlock ||
             kArithExpr + kIf + kArithExpr + kElse + kArithExpr || kArithExpr,

    kBlock |= kLBrace + kStmt * Any() + kRBrace,

    kStmt |= kReturn + kExpr + kSemicolon || kPrint + kExpr + kSemicolon ||
             kWhile + kExpr + kBlock ||
             kIf + kExpr + kBlock + (kElse + kBlock) * Optional() ||
             kFor + kStmt + kExpr + kSemicolon + kStmt + kBlock ||
             kDef + kId + kLParen + kVarList * Optional() + kRParen + kBlock ||
             kExpr + kSemicolon || kExpr + kEq + kExpr + kSemicolon,
    kPgm |= kStmt * AtLeast<1>());

template <>
struct Converter<SteinLang, Var<VariableTag::VARLIST>> {
  using in_type = ConcreteType<SteinLang, Var<VariableTag::VARLIST>>;
  using out_type = std::vector<std::string>;
  static std::vector<std::string> Convert(in_type in) {
    std::vector<std::string> ret;
    ret.push_back(std::move(std::get<0>(*in)));
    for (auto& comma_id : std::get<1>(*in)) {
      ret.push_back(std::move(std::get<1>(comma_id)));
    }
    return ret;
  }
};

using SlTokenizer = Tokenizer<TokenTag>;
using SlToken = SlTokenizer::Token;
using SlTokenIt = std::vector<SlToken>::iterator;

void TestParseVarList(SlTokenIt begin, SlTokenIt end) {
  auto result = Parser<SteinLang, Var<VariableTag::VARLIST>>::Parse(begin, end, true);
  if (!result.is_success()) {
    return;
  }
  std::vector<std::string> vars = result.value();
}

void TestParseStuff(SlTokenIt begin, SlTokenIt end) {
  Parser<SteinLang, Var<VariableTag::MONOP>>::Parse(begin, end, true);
  Parser<SteinLang, Var<VariableTag::BINOP>>::Parse(begin, end, true);
  Parser<SteinLang, Var<VariableTag::EXPR>>::Parse(begin, end, true);
  Parser<SteinLang, Var<VariableTag::PGM>>::Parse(begin, end, true);
}

}  // namespace lang
