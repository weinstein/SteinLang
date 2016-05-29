#ifndef _FILE_IO_H
#define _FILE_IO_H

#include <fstream>
#include <google/protobuf/text_format.h>
#include <sstream>
#include <string>

#include "proto/language.pb.h"

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

#endif  // _FILE_IO_H
