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

auto run_main(int argc, char const* argv[]) -> int {

  static constexpr char const* config_file{"/Users/adamc/github/dropclone/config/dropclone.json"};
  
  logger.get(logger_id::core)->info("dropclone starting...");
  drop_clone clone{config_file, nlohmann_json_parser{}};
  logger.get(logger_id::core)->info("dropclone terminated.");
  
  return EXIT_SUCCESS;
}