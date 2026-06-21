#include <dropclone/nlohmann_json_parser.hpp>
#include <dropclone/drop_clone.hpp>
#include <dropclone/logger_manager.hpp>
#include <dropclone/exception.hpp>
#include <dropclone/errorcode.hpp>
#include <dropclone/utility.hpp>
#include <atomic>
#include <filesystem>
#include <cstdlib>
#include <csignal>
#include <chrono>
#include <string_view>

using dropclone::logger;
using dropclone::logger_id;
using dropclone::drop_clone;
using dropclone::nlohmann_json_parser;

namespace fs = std::filesystem;
namespace dc = dropclone;

namespace {

  constexpr auto sync_interval_seconds{30};
  std::atomic_bool running{true};

auto get_config_path(int argc, char const *argv[]) -> fs::path {
  if (argc < 2) {
    dc::throw_exception<dc::errorcode::cli>(
      dc::errorcode::cli::missing_config_file_argument
    );
  }

  std::string_view arg{argv[1]};
  std::string param{"--config_file"};
  if (!arg.starts_with(param)) {
    dc::throw_exception<dc::errorcode::cli>(
      dc::errorcode::cli::invalid_config_file_argument,
      param.append("=<path>"),
      arg
    );
  }

  return arg.substr(param.size()+1);
}

auto register_signal_handler() -> void {
   auto const signal_handler = [](int) -> void {
    logger.get(logger_id::core)->info(
      dc::utility::formatter<dc::messagecode::system>::format(
        dc::messagecode::system::termination_requested_by_signal,
        sync_interval_seconds
      )
    );
    running.store(false);
  };

  if (std::signal(SIGINT, signal_handler) == SIG_ERR) {
    dc::throw_exception<dc::errorcode::signal>(
      dc::errorcode::signal::sigint_handler_registration_failed
    );
  }

  if (std::signal(SIGTERM, signal_handler) == SIG_ERR) {
    dc::throw_exception<dc::errorcode::signal>(
      dc::errorcode::signal::sigterm_handler_registration_failed
    );
  }
}

}

auto run_main(int argc, char const* argv[]) -> int {
  logger.get(logger_id::core)->info(
  dc::utility::formatter<dc::messagecode::system>::format(
    dc::messagecode::system::application_starting
  ));

  try {
    register_signal_handler();
    auto const config_file = get_config_path(argc, argv);
    drop_clone clone{config_file, nlohmann_json_parser{}};
    while (running.load()) {
      clone.sync();
      std::this_thread::sleep_for(
        std::chrono::seconds{sync_interval_seconds}
      );
    }
  } catch (dc::exception const& e) {
    logger.get(logger_id::core)->error(
      dc::utility::formatter<dc::errorcode::system>::format(
        dc::errorcode::system::application_terminated,
        e.what()
    ));
    return EXIT_FAILURE;
  }

  logger.get(logger_id::core)->info(
  dc::utility::formatter<dc::messagecode::system>::format(
    dc::messagecode::system::application_terminating
  ));

  return EXIT_SUCCESS;
}