// Some quick and dirty utils for reading data from files.

#ifndef UTIL_FILE_IO_H_
#define UTIL_FILE_IO_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

namespace util {

std::string ReadToString(std::istream* stream) {
  std::stringstream buf;
  buf << stream->rdbuf();
  return buf.str();
}

std::string ReadFileToString(const std::string& fname) {
  std::ifstream stream(fname.c_str());
  return ReadToString(&stream);
}

std::string ReadStdInToString() { return ReadToString(&std::cin); }

std::vector<std::string> ReadFileLines(const std::string& fname) {
  std::ifstream fs(fname);
  std::vector<std::string> result;
  for (std::string line; std::getline(fs, line);) {
    result.push_back(line);
  }
  return result;
}

}  // util

#endif  // UTIL_FILE_IO_H_
