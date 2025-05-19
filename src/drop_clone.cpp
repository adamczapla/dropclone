#include <spdlog/spdlog.h>
#include <spdlog/logger.h>
#include "spdlog/async.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "spdlog/sinks/rotating_file_sink.h"
#include <dropclone/drop_clone.hpp>
#include <dropclone/clone_config.hpp>
#include <dropclone/clone_manager.hpp>
#include <dropclone/logger_manager.hpp>
#include <dropclone/utility.hpp>
#include <dropclone/exception.hpp>
#include <dropclone/messagecode.hpp>
#include <utility>
#include <filesystem>
#include <string>
#include <ranges>
#include <vector>

namespace dropclone {

namespace rng = std::ranges;
namespace dc = dropclone;

drop_clone::drop_clone(fs::path config_path, config_parser parser) { 
  try {
    init_config_logger();

    clone_config_ = parser(config_path);
    logger.get(logger_id::config)->info(
      utility::formatter<messagecode::config>::format(
        messagecode::config::config_file_parsed,
        config_path.string()
    ));

    clone_config_.sanitize(config_path);
    clone_config_.validate();
    logger.get(logger_id::config)->info(
      utility::formatter<messagecode::config>::format(
        messagecode::config::config_validated,
        clone_config_.entries.size()
    ));

    rng::for_each(clone_config_.entries, [&](auto const& entry) {
      managers_.emplace_back(entry);
    });

    spdlog::init_thread_pool(8192, 1);
    init_sync_logger();

    logger.get(logger_id::config)->info("ready for use.");

  } catch (dropclone::exception const& e) {
    logger.get(logger_id::config)->error(e.what());
    throw;
  } catch (spdlog::spdlog_ex const& e) {
    logger.get(logger_id::config)->error(
      utility::formatter<errorcode::logger>::format(
        errorcode::logger::initialization_failed, 
        to_string(logger_id::config), e.what()
    ));
    throw_exception<errorcode::logger>(
      errorcode::logger::initialization_failed,
      to_string(logger_id::config), e.what()
    );
  } catch (std::exception const& e) {
    logger.get(logger_id::config)->error(
      utility::formatter<errorcode::system>::format(
        errorcode::system::unhandled_std_exception, e.what()
    ));
    throw_exception<errorcode::system>(
      errorcode::system::unhandled_std_exception, e.what() 
    );
  } catch (...) {
    logger.get(logger_id::config)->error(
      utility::formatter<errorcode::system>::format(
        errorcode::system::unknown_fatal_error
    ));
    throw_exception<errorcode::system>(
      errorcode::system::unknown_fatal_error
    );
  }
}

auto drop_clone::init_config_logger() -> void {
  logger.add(logger_id::config, [] {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_pattern(logger_manager::default_pattern);
    console_sink->set_level(spdlog::level::info);
    auto logger = std::make_shared<spdlog::logger>(to_string(logger_id::config), console_sink);
    logger->set_level(spdlog::level::trace);
    return logger;
  }());
}

auto drop_clone::init_sync_logger() -> void {
  auto const log_file = clone_config_.log_directory / "sync_log.txt";

  logger.add(logger_id::sync, [&] {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::critical);
    console_sink->set_pattern(logger_manager::default_pattern);

    constexpr auto max_file_size = 1024*1024*10;
    constexpr auto max_log_files = 3;

    auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
      log_file.string(), max_file_size, max_log_files);
    // TODO: set rotating_sink log level to spdlog::level::info before final release
    // ! currently set to debug for development purposes
    rotating_sink->set_level(spdlog::level::debug);
    rotating_sink->set_pattern(logger_manager::default_pattern);

    auto logger = std::make_shared<spdlog::async_logger>(
      to_string(logger_id::sync), spdlog::sinks_init_list{rotating_sink, console_sink}, 
      spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    logger->set_level(spdlog::level::trace);
    spdlog::register_logger(logger);

    return logger;
  }()); 

  logger.get(logger_id::config)->info(
    utility::formatter<messagecode::config>::format(
      messagecode::config::logging_ready,
      log_file.string()
  ));
}

auto drop_clone::sync() -> void {
  try {
    rng::for_each(managers_, [&](auto& clone_manager) { 
      try {
        clone_manager.sync(); 
      } catch (dc::exception const& err) {
        logger.get(logger_id::sync)->error(
          utility::formatter<errorcode::sync>::format(
            errorcode::sync::sync_failed, 
            err.what()
        ));
      }
    });
  } catch (std::exception const& e) {
      logger.get(logger_id::config)->error(
        utility::formatter<errorcode::system>::format(
          errorcode::system::unhandled_std_exception, e.what()
      ));
      throw_exception<errorcode::system>(
        errorcode::system::unhandled_std_exception, e.what() 
      );
  } catch (...) {
    logger.get(logger_id::config)->error(
      utility::formatter<errorcode::system>::format(
        errorcode::system::unknown_fatal_error
    ));
    throw_exception<errorcode::system>(
      errorcode::system::unknown_fatal_error
    );
  }
}
  
} // namespace dropclone