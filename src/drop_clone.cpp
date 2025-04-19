#include <spdlog/spdlog.h>
#include <spdlog/logger.h>
#include "spdlog/async.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "spdlog/sinks/rotating_file_sink.h"
#include <dropclone/drop_clone.hpp>
#include <dropclone/clone_config.hpp>
#include <dropclone/logger_manager.hpp>
#include <dropclone/utility.hpp>
#include <dropclone/exception.hpp>
#include <dropclone/messagecode.hpp>
#include <utility>
#include <filesystem>
#include <string>

namespace dropclone {

drop_clone::drop_clone(fs::path config_path, config_parser parser)  
    : config_path_{std::move(config_path)}, parser_{std::move(parser)} { 
  try {
    init_config_logger();

    clone_config_ = parser_(config_path_);
    logger.get(logger_id::config)->info(
      utility::formatter<messagecode::logger::config>::format(
        messagecode::logger::config::config_file_parsed,
        config_path_.string()
    ));

    for (auto& entry : clone_config_.entries) { entry.sanitize(); }
    clone_config_.sanitize(config_path_);
    clone_config_.validate();
    logger.get(logger_id::config)->info(
      utility::formatter<messagecode::logger::config>::format(
        messagecode::logger::config::config_validated,
        clone_config_.entries.size()
    ));

    spdlog::init_thread_pool(8192, 1);
    init_startup_logger();
    init_daemon_logger();

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

auto drop_clone::init_startup_logger() -> void {
  auto const log_file = clone_config_.log_directory / "startup_log.txt";

  logger.add(logger_id::startup, [&] {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::info);
    console_sink->set_pattern(logger_manager::default_pattern);

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file.string(), false);
    file_sink->set_level(spdlog::level::info);
    file_sink->set_pattern(logger_manager::default_pattern);

    auto logger = std::make_shared<spdlog::async_logger>(
      to_string(logger_id::startup), spdlog::sinks_init_list{console_sink, file_sink}, 
      spdlog::thread_pool(), spdlog::async_overflow_policy::block
    );

    logger->set_level(spdlog::level::trace);
    spdlog::register_logger(logger);

    return logger;
  }());

  logger.get(logger_id::config)->info(
    utility::formatter<messagecode::logger::config>::format(
      messagecode::logger::config::logging_ready,
      log_file.string()
  ));
}

auto drop_clone::init_daemon_logger() -> void {
  auto const log_file = clone_config_.log_directory / "daemon_log.txt";

  logger.add(logger_id::daemon, [&] {
    constexpr auto max_file_size = 1024*1024*10;
    constexpr auto max_log_files = 3;

    auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
      log_file.string(), max_file_size, max_log_files);
    rotating_sink->set_level(spdlog::level::info);
    rotating_sink->set_pattern(logger_manager::default_pattern);

    auto logger = std::make_shared<spdlog::async_logger>(
      to_string(logger_id::daemon), spdlog::sinks_init_list{rotating_sink}, 
      spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    logger->set_level(spdlog::level::trace);
    spdlog::register_logger(logger);

    return logger;
  }()); 

  logger.get(logger_id::config)->info(
    utility::formatter<messagecode::logger::config>::format(
      messagecode::logger::config::logging_ready,
      log_file.string()
  ));
}
  
} // namespace dropclone