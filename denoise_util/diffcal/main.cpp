#include "diffcal.hpp"

int main(int argc, const char **argv) {
  
  if (argc != 3 || (!is_format(argv[1]) || !is_format(argv[2]))) {
    std::cerr << "Must call diffcal with two arguments: ./diffcal <format1> <format2>, where each format takes an integer, e.g. image%00d.png" << std::endl;
    exit(-1);
  }

  const int start_value = 0;
  
  ImageFileIterator imit(start_value, argv[1], argv[2]);

  DiffResultState result = computeDiff(imit);

  outputResult(result, "diff_results.json");
}

