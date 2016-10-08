#ifndef UTIL_STRINGS_H_
#define UTIL_STRINGS_H_

#include <string>

namespace util {

inline bool HasPrefix(const std::string& str, const std::string& prefix) {
  return str.compare(0, prefix.length(), prefix) == 0;
}

inline void EscapeForRegex(std::string* str) {
  for (auto it = str->begin(); it != str->end(); ++it) {
    switch (*it) {
      case '{':
      case '}':
      case '(':
      case ')':
      case '[':
      case ']':
      case '$':
      case '*':
      case '.':
      case '?':
      case '+':
      case '|':
      case '^':
      case '\\':
        it = str->insert(it, '\\') + 1;
        break;
      default:
        break;
    }
  }
}

inline void Unescape(std::string* str) {
  for (auto it = str->begin(); it != str->end() && it != str->end() - 1; ++it) {
    if (*it == '\\') {
      it = str->erase(it);
      switch (*it) {
        case 'n':
          *it = '\n';
          break;
        case 't':
          *it = '\t';
          break;
        case '0':
          *it = '\0';
          break;
        default:
          break;
      }
    }
  }
}

}  // namespace util

#endif  // UTIL_STRINGS_H_
