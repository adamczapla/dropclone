#include <drop_clone.hpp>
#include <clone_config.hpp>
#include <logger_manager.hpp>
#include <utility.hpp>
#include <exception.hpp>
#include <messagecode.hpp>
#include <utility>
#include <fstream>
#include <filesystem>
#include <string>
#include <string_view>

namespace dropclone {

drop_clone::drop_clone(fs::path config_path, config_parser parser)  
    : config_path_{std::move(config_path)}, parser_{std::move(parser)} { 
  try {
    init_startup_logger();

    clone_config_ = parser_(config_path_);
    logger.get(logger_id::startup)->info(
      utility::formatter<messagecode::logger::startup>::format(
        messagecode::logger::startup::config_file_parsed,
        config_path_.string()
    ));

    for (auto& entry : clone_config_.entries) { entry.sanitize(); }
    clone_config_.sanitize(config_path_);
    clone_config_.validate();
    logger.get(logger_id::startup)->info(
      utility::formatter<messagecode::logger::startup>::format(
        messagecode::logger::startup::config_validated,
        clone_config_.entries.size()
    ));

    spdlog::init_thread_pool(8192, 1);
    init_task_logger();
    init_daemon_logger();

    logger.get(logger_id::startup)->info("ready for use.");

  } catch (dropclone::exception const& e) {
    logger.get(logger_id::startup)->error(e.what());
    std::exit(EXIT_FAILURE);
  } catch (spdlog::spdlog_ex const& e) {
    logger.get(logger_id::startup)->error(
      utility::formatter<errorcode::logger>::format(
        errorcode::logger::initialization_failed,
        to_string(logger_id::startup), e.what()
    ));
    std::exit(EXIT_FAILURE);
  } catch (std::exception const& e) {
    logger.get(logger_id::startup)->error(
      utility::formatter<errorcode::system>::format(
        errorcode::system::unhandled_std_exception, e.what()
    ));
    std::exit(EXIT_FAILURE);
  } catch (...) {
    logger.get(logger_id::startup)->error(
      utility::formatter<errorcode::system>::format(
        errorcode::system::unknown_fatal_error)
    );
    std::exit(EXIT_FAILURE);
  }
}

auto drop_clone::init_startup_logger() -> void {
  logger.add(logger_id::startup, [] {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v");
    console_sink->set_level(spdlog::level::trace);
    auto logger = std::make_shared<spdlog::logger>(to_string(logger_id::startup), console_sink);
    logger->set_level(spdlog::level::info);
    return logger;
  }());
}

auto drop_clone::init_task_logger() -> void {
  auto const log_file = clone_config_.log_directory / "task_log.txt";

  logger.add(logger_id::task, [&] {
    std::string const pattern{"[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v"};

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::trace);
    console_sink->set_pattern(pattern);

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file.string(), true);
    file_sink->set_level(spdlog::level::trace);
    file_sink->set_pattern(pattern);

    auto logger = std::make_shared<spdlog::async_logger>(
      to_string(logger_id::task), spdlog::sinks_init_list{console_sink, file_sink}, 
      spdlog::thread_pool(), spdlog::async_overflow_policy::block
    );

    logger->set_level(spdlog::level::info);
    spdlog::register_logger(logger);

    return logger;
  }());

  logger.get(logger_id::startup)->info(
    utility::formatter<messagecode::logger::startup>::format(
      messagecode::logger::startup::logging_ready,
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
    rotating_sink->set_level(spdlog::level::trace);
    rotating_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v");

    auto logger = std::make_shared<spdlog::async_logger>(
      to_string(logger_id::daemon), spdlog::sinks_init_list{rotating_sink}, 
      spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    logger->set_level(spdlog::level::info);
    spdlog::register_logger(logger);

    return logger;
  }()); 

  logger.get(logger_id::startup)->info(
    utility::formatter<messagecode::logger::startup>::format(
      messagecode::logger::startup::logging_ready,
      log_file.string()
  ));
}
  
} // namespace dropclone