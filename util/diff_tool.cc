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
      std::cout << "[" << mod.addition().insert_pos - lhs.begin() << "]\t + "
                << *mod.addition().data << "\n";
    }
    if (mod.is_deletion()) {
      std::cout << "[" << mod.deletion().data - lhs.begin() << "]\t - "
                << *mod.deletion().data << "\n";
    }
  }
  return 0;
}
