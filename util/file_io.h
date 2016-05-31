// Some quick and dirty utils for reading data from files.

#ifndef FILE_IO_H__
#define FILE_IO_H__

#include <fstream>
#include <google/protobuf/text_format.h>
#include <sstream>
#include <string>

#include "proto/language.pb.h"

namespace steinlang {

std::string ReadFileToString(const std::string& fname) {
  std::stringstream buf;
  buf << std::ifstream(fname.c_str()).rdbuf();
  return buf.str();
}

void ParseAsciiMessage(const std::string& fname, google::protobuf::Message* msg) {
  const std::string txt = ReadFileToString(fname);
  google::protobuf::TextFormat::ParseFromString(txt, msg);
}

steinlang::Program ParseAsciiProgram(const std::string& fname) {
  steinlang::Program pgm;
  ParseAsciiMessage(fname, &pgm);
  return pgm;
}

}  // steinlang

#endif  // FILE_IO_H__
