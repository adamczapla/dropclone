#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <clone_manager.hpp>

auto main(int argc, char const* argv[]) -> int {
  namespace fs = std::filesystem;
  namespace dc = dropclone;

  std::cout << "test\n";

  std::cout << fs::current_path() << '\n';
  fs::path p{"dropclone"};
  dc::path_info fi{fs::last_write_time(p), fs::file_size(p), fs::status(p).permissions()};
  std::cout << std::hash<dc::path_info>{}(fi) << '\n';
  
  return EXIT_SUCCESS;
}