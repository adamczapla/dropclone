#include <drop_clone.hpp>
#include <logger_manager.hpp>
#include <parsers/nlohmann_json_parser.hpp>

#include <iostream>
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;
namespace dc = dropclone;

using json = nlohmann::json;
using dropclone::logger;
using dropclone::logger_id;
using dropclone::drop_clone;
using dropclone::nlohmann_json_parser;

auto main(int argc, char const* argv[]) -> int {

  static constexpr char const* config_file{"/Users/adamc/github/dropclone/config/dropclone.json"};
  
  logger.get(logger_id::core)->info("dropclone starting...");
  drop_clone clone{config_file, nlohmann_json_parser{}};
  logger.get(logger_id::core)->info("dropclone terminated.");

  // std::cout << fs::current_path() << '\n';
  // fs::path p{"dropclone"};
  // dc::path_info fi{fs::last_write_time(p), fs::file_size(p), fs::status(p).permissions()};
  // std::cout << std::hash<dc::path_info>{}(fi) << '\n';
  
  return EXIT_SUCCESS;
}