#pragma once

#include <unordered_map>
#include <string_view>

namespace dropclone::messagecode {
  
struct config {
  static constexpr auto config_file_parsed  = "config_message.001";
  static constexpr auto config_validated    = "config_message.002";
  static constexpr auto logging_ready       = "config_message.003";

  static inline std::unordered_map<std::string_view, std::string_view> const messages{
    {config_file_parsed, "Configuration file '{}' successfully parsed"},
    {config_validated, "Configuration validated successfully – {} clone entries ready"},
    {logging_ready, "Logging initialized to: {}"}
  };
};

struct command {
  static constexpr auto enter_command    = "command_message.001";
  static constexpr auto leave_command    = "command_message.002";

  static constexpr auto copy_file        = "command_message.003";
  static constexpr auto rename_file      = "command_message.004";
  static constexpr auto remove_file      = "command_message.005";

  static constexpr auto create_directory = "command_message.006";
  static constexpr auto remove_directory = "command_message.007";

  static constexpr auto execute_skipped  = "command_message.008";
  static constexpr auto undo_skipped     = "command_message.009";

  static inline std::unordered_map<std::string_view, std::string_view> const messages{
    {enter_command, "Enter {}::{}:"},
    {leave_command, "Leave {}::{}."},
    {copy_file, "Copy file '{}' -> '{}'"},
    {rename_file, "Rename file: '{}' -> '{}'"},
    {remove_file, "Remove file: '{}'"},
    {create_directory, "Create directory: '{}'"},
    {remove_directory, "Remove directory: '{}'"},
    {execute_skipped, "'{}::execute' skipped due to unsafe state"},
    {undo_skipped, "'{}::undo' skipped – no recovery required"}
  };
};

struct system {
  static constexpr auto application_starting            = "system_message.001";
  static constexpr auto application_terminating         = "system_message.002";
  static constexpr auto termination_requested_by_signal = "system_message.003";

  static inline std::unordered_map<std::string_view, std::string_view> const messages{
    {application_starting, "dropclone starting..."},
    {application_terminating, "dropclone terminated."},
    {termination_requested_by_signal, "Termination requested – dropclone will stop in at most {} seconds"}
  };
};

} // namespace dropclone::messagecode