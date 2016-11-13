#include <gflags/gflags.h>
#include <iostream>
#include <string>
#include <vector>

#include "util/diff.h"
#include "util/file_io.h"

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  if (argc != 3) {
    std::cout << "usage: " << argv[0] << " file1 file2\n";
    return 1;
  }

  const std::vector<std::string> lhs = util::ReadFileLines(argv[1]);
  const std::vector<std::string> rhs = util::ReadFileLines(argv[2]);
  const auto difference = util::DiffSeqs(lhs, rhs);
  for (const auto& mod : difference) {
    if (mod.is_addition()) {
      std::cout << "[" << mod.lhs - lhs.begin() << "]\t + " << *mod.rhs << "\n";
    }
    if (mod.is_deletion()) {
      std::cout << "[" << mod.lhs - lhs.begin() << "]\t - " << *mod.lhs << "\n";
    }
    if (!mod.is_change() && mod.lhs != lhs.end()) {
      std::cout << "[" << mod.lhs - lhs.begin() << "]\t = " << *mod.lhs << "\n";
    }
  }
  return 0;
}
