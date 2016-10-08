#ifndef LANG_TOKENIZER_H_
#define LANG_TOKENIZER_H_

#include <iterator>
#include <regex>
#include <utility>
#include <vector>

namespace lang {

template <typename TokenTag>
class Tokenizer {
 public:
  struct Token {
    TokenTag tag;
    std::string value;
    std::iterator_traits<std::string::const_iterator>::difference_type pos;
  };

  void add_rule(TokenTag tag, std::string rule) {
    token_rules_.emplace_back(std::move(tag), std::regex{std::move(rule)});
  }

  void ignore_rule(std::string rule) {
    ignore_rules_.emplace_back(std::regex{std::move(rule)});
  }

  std::vector<Token> operator()(std::string text) const {
    std::vector<Token> result;
    for (auto it = text.cbegin(); it != text.cend();
         it = FindOneMatch(text.cbegin(), it, text.cend(), &result)) {
    }
    return result;
  }

 private:
  std::string::const_iterator FindOneMatch(std::string::const_iterator begin,
                                           std::string::const_iterator start,
                                           std::string::const_iterator end,
                                           std::vector<Token>* output) const {
    for (const std::regex& ignore : ignore_rules_) {
      std::smatch match;
      if (std::regex_search(start, end, match, ignore,
                            std::regex_constants::match_continuous) && !match.str().empty()) {
        return match[0].second;
      }
    }
    for (const auto& kv : token_rules_) {
      std::smatch match;
      if (std::regex_search(start, end, match, kv.second,
                            std::regex_constants::match_continuous) && !match.str().empty()) {
        output->push_back(
            {kv.first, match.str(), match.position() + (start - begin)});
        return match[0].second;
      }
    }
    std::string rest(start, end);
    printf("no match found for: `%s`\n", rest.c_str());
    return end;
  }

  std::vector<std::pair<TokenTag, std::regex>> token_rules_;
  std::vector<std::regex> ignore_rules_;
};

}  // namespace lang

#endif  // LANG_TOKENIZER_H_
