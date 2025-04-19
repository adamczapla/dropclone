#pragma once

#include <unordered_map>
#include <string_view>

namespace dropclone::errorcode {

struct config {
  static constexpr auto file_not_found            = "config_error.001";
  static constexpr auto parse_error               = "config_error.002";
  static constexpr auto conversion_error          = "config_error.003";
  static constexpr auto path_not_absolute         = "config_error.004";
  static constexpr auto invalid_clone_mode        = "config_error.005";
  static constexpr auto overlapping_path_conflict = "config_error.006";
  static constexpr auto path_not_configured       = "config_error.007";
  
  static inline std::unordered_map<std::string_view, std::string_view> const messages{
    {file_not_found, "cannot open config file: {}"},
    {parse_error, "could not parse config file {} |\n↳ origin error: \n\t↳ {}"},
    {conversion_error, "conversion error |\n↳ origin error: \n\t↳ {}"},
    {path_not_absolute, "'{}' must be an absolte path"},
    {invalid_clone_mode, "'{}' must be (copy or move)"},
    {overlapping_path_conflict, "Overlapping path detected in '{}': {}"},
    {path_not_configured, "{} path is not configured or not absolute – using fallback: '{}'"}
  };
};

struct filesystem {
  static constexpr auto could_not_create_directory = "filesystem_error.001";
  
  static inline std::unordered_map<std::string_view, std::string_view> const messages{
    {could_not_create_directory, "could not create directory: {} |\n↳ origin error: \n\t↳ {}"}
  };
};

struct logger {
  static constexpr auto logger_id_not_found   = "logger_error.001";
  static constexpr auto initialization_failed = "logger_error.002";

  static inline std::unordered_map<std::string_view, std::string_view> const messages{
    {logger_id_not_found, "logger_id '{}' not registered – using 'core' logger as fallback."},
    {initialization_failed, "Failed to initialize logger {} |\n↳ origin error:\n\t↳ {}"}
  };
};

struct system {
  static constexpr auto unhandled_std_exception = "system_error.001";
  static constexpr auto unknown_fatal_error     = "system_error.002";

  static inline std::unordered_map<std::string_view, std::string_view> const messages{
    {unhandled_std_exception, "Unhandled std::exception occurred |\n↳ origin error:\n\t↳ {}"},
    {unknown_fatal_error, "Unknown fatal error occurred – possible internal crash or signal"}
  };
};
  
} // namespace dropclone::errorcode