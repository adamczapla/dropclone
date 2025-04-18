#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/logger.h>
#include "spdlog/async.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "spdlog/sinks/rotating_file_sink.h"
#include <errorcode.hpp>
#include <utility.hpp>
#include <unordered_map>
#include <memory>
#include <utility>
#include <shared_mutex>

namespace dropclone {

enum class logger_id { core, config, startup, daemon };

inline auto to_string(logger_id id) -> std::string {
  switch (id) {
    case logger_id::core:     return "core"; 
    case logger_id::config:   return "config";  
    case logger_id::startup:  return "startup"; 
    case logger_id::daemon:   return "daemon";
  }
  return "unknown";
}

class logger_manager {
 public:
  static inline auto const default_pattern{"[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v"};

  logger_manager() : core_logger_{
    [] {
      auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
      sink->set_pattern(default_pattern);
      sink->set_level(spdlog::level::trace);
      auto logger = std::make_shared<spdlog::logger>(to_string(logger_id::core), sink);
      logger->set_level(spdlog::level::info);
      return logger;
    }()
  } {}

  explicit logger_manager(std::unique_ptr<spdlog::logger> core_logger)
    : core_logger_{std::move(core_logger)}
  {}

  auto get(logger_id id) -> std::shared_ptr<spdlog::logger> {
    if (id == logger_id::core) { return core_logger_; }
    try {
      std::shared_lock<std::shared_mutex> logger_guard{logger_mutex_};
      return loggers_.at(id);
    } catch (std::out_of_range const& e) {
      core_logger_->warn(
        utility::formatter<errorcode::logger>::format(
          errorcode::logger::logger_id_not_found, to_string(id)
        )
      );
      return core_logger_;
    }
  }

  auto add(logger_id id, std::shared_ptr<spdlog::logger> logger) -> logger_manager& {
    std::lock_guard<std::shared_mutex> logger_guard{logger_mutex_};
    loggers_.emplace(id, logger);
    return *this;
  }

 private:
  std::shared_mutex logger_mutex_;
  std::shared_ptr<spdlog::logger> core_logger_;
  std::unordered_map<logger_id, std::shared_ptr<spdlog::logger>> loggers_;
};

inline logger_manager logger{};
  
} // namespace dropclone