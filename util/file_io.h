// Some quick and dirty utils for reading data from files.

#ifndef UTIL_FILE_IO_H_
#define UTIL_FILE_IO_H_

#include <fstream>
#include <sstream>
#include <string>

namespace util {

std::string ReadFileToString(const std::string& fname) {
  std::stringstream buf;
  buf << std::ifstream(fname.c_str()).rdbuf();
  return buf.str();
}

}  // util

#endif  // UTIL_FILE_IO_H_
