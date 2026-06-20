#include <dropclone/nlohmann_json_parser.hpp>
#include <dropclone/drop_clone.hpp>
#include <dropclone/logger_manager.hpp>
#include <dropclone/exception.hpp>
#include <dropclone/errorcode.hpp>
#include <dropclone/utility.hpp>
#include <ranges>
#include <filesystem>
#include <cstdlib>
#include <chrono>

using dropclone::logger;
using dropclone::logger_id;
using dropclone::drop_clone;
using dropclone::nlohmann_json_parser;

namespace rng = std::ranges;
namespace vws = std::views;
namespace util = dropclone::utility;
namespace fs = std::filesystem;
namespace dc = dropclone;

auto get_config_path(int argc, char const *argv[]) -> fs::path {
  if (argc < 2) {
    dc::throw_exception<dc::errorcode::cli>(
      dc::errorcode::cli::missing_config_file_argument
    );
  }

  std::string_view arg{argv[1]};
  constexpr std::string_view param{"--config_file"};
  if (!arg.starts_with(param)) {
    dc::throw_exception<dc::errorcode::cli>(
      dc::errorcode::cli::invalid_config_file_argument,
      arg
    );
  }

  return arg.substr(param.size()+1);
}

auto run_main(int argc, char const* argv[]) -> int {
  logger.get(logger_id::core)->info("dropclone starting...");

  try {
    auto const config_file = get_config_path(argc, argv);
    drop_clone clone{config_file, nlohmann_json_parser{}};
    clone.sync();
    std::this_thread::sleep_for(std::chrono::seconds{30});
    clone.sync();
  } catch (dc::exception const& e) {
    logger.get(logger_id::core)->error(e.what());
  }
  logger.get(logger_id::core)->info("dropclone terminated.");
  
  return EXIT_SUCCESS;
}