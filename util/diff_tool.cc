#include <gflags/gflags.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "util/diff.h"

namespace {

std::vector<std::string> ReadFileLines(const std::string& fname) {
  std::ifstream fs(fname);
  std::vector<std::string> result;
  for (std::string line; std::getline(fs, line); ) {
    result.push_back(line);
  }
  return result;
}

}  // namespace

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  if (argc != 3) {
    std::cout << "usage: " << argv[0] << " file1 file2\n";
    return 1;
  }

  const std::vector<std::string> lhs = ReadFileLines(argv[1]);
  const std::vector<std::string> rhs = ReadFileLines(argv[2]);
  const auto difference = util::Diff(lhs, rhs);
  for (const auto& mod : difference) {
    if (mod.is_addition()) {
      std::cout << "[" << mod.addition().insert_pos - lhs.begin() << "]\t + "
                << *mod.addition().data << "\n";
    }
    if (mod.is_deletion()) {
      std::cout << "[" << mod.deletion().delete_pos - lhs.begin() << "]\t - "
                << *mod.deletion().delete_pos << "\n";
    }
  }
  return 0;
}
