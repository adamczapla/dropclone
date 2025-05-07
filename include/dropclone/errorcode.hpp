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
  static constexpr auto missing_required_field    = "config_error.008";
  static constexpr auto no_entries_defined        = "config_error.009";
  static constexpr auto invalid_field_type        = "config_error.010";
  
  static inline std::unordered_map<std::string_view, std::string_view> const messages{
    {file_not_found, "cannot open config file: {}"},
    {parse_error, "could not parse config file {} |\n↳ origin error: \n\t↳ {}"},
    {conversion_error, "conversion error |\n↳ origin error: \n\t↳ {}"},
    {path_not_absolute, "'{}' must be an absolte path"},
    {invalid_clone_mode, "'{}' must be (copy or move)"},
    {overlapping_path_conflict, "Overlapping path detected in '{}': {}"},
    {path_not_configured, "{} path is not configured or not absolute – using fallback: '{}'"},
    {missing_required_field, "missing required field: '{}' in config file '{}'"},
    {no_entries_defined, "no entries defined in config file '{}'"},
    {invalid_field_type, "field '{}' has invalid type"}
  };
};

struct filesystem {
  static constexpr auto could_not_create_directory    = "filesystem_error.001";
  static constexpr auto failed_to_traverse_directory  = "filesystem_error.002";
  
  static inline std::unordered_map<std::string_view, std::string_view> const messages{
    {could_not_create_directory, "could not create directory: {} |\n↳ origin error: \n\t↳ {}"},
    {failed_to_traverse_directory, "failed to traverse directory: {} in {} |\n↳ origin error: \n\t↳ {}"}
  };
};

struct command {
  static constexpr auto copy_command_failed           = "command_error.001";
  static constexpr auto rename_command_failed         = "command_error.002";
  static constexpr auto remove_command_failed         = "command_error.003";
  
  static inline std::unordered_map<std::string_view, std::string_view> const messages{
    {copy_command_failed, "copy_command::{}: '{}' → '{}' failed |\n↳ origin error:\n\t↳ {}"},
    {rename_command_failed, "rename_command::{}: '{}' → '{}' failed |\n↳ origin error:\n\t↳ {}"},
    {remove_command_failed, "remove_command::{}: '{}' failed |\n↳ origin error:\n\t↳ {}"}
  };
};

struct transaction {
  static constexpr auto start_failed            = "transaction_error.001";
  static constexpr auto rollback_failed         = "transaction_error.002";
  static constexpr auto unrecovered_entries     = "transaction_error.003";
  static constexpr auto unrecovered_file        = "transaction_error.004";
  static constexpr auto unrecovered_directory   = "transaction_error.005";

  static inline std::unordered_map<std::string_view, std::string_view> const messages{
    {start_failed, "transaction failed and was rolled back |\n↳ origin error:\n\t↳ {}"},
    {rollback_failed, "rollback failed – system may be inconsistent |\n↳ origin error:\n\t↳ {}"},
    {unrecovered_entries, "Unrecovered entries remain in snapshot '{}'"},
    {unrecovered_file, "Unrecovered file: '{}'"},
    {unrecovered_directory, "Unrecovered directory: '{}'"}
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

struct test {
  static constexpr auto failed_prepare_readonly_dir   = "test_error.001";
  static constexpr auto could_not_open_temporary_file = "test_error.002";

  static inline std::unordered_map<std::string_view, std::string_view> const messages{
    {failed_prepare_readonly_dir, "Could not prepare readonly directory: {} |\n↳ origin error:\n\t↳ {}"},
    {could_not_open_temporary_file, "cannot open temporary config file: {}"},
  };
};
  
} // namespace dropclone::errorcode