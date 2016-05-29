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

language::Program ParseAsciiProgram(const std::string& fname) {
  const std::string pgm_txt = ReadFileToString(fname);
  language::Program pgm;
  google::protobuf::TextFormat::ParseFromString(pgm_txt, &pgm);
  return pgm;
}

}  // steinlang

#endif  // FILE_IO_H__
