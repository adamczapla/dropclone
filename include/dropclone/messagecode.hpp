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

  static constexpr auto undo_before_execute = "command_message.008";
  static inline std::unordered_map<std::string_view, std::string_view> const messages{
    {enter_command, "Enter {}::{}:"},
    {leave_command, "Leave {}::{}."},
    {copy_file, "Copy file '{}' -> '{}'"},
    {rename_file, "Rename file: '{}' -> '{}'"},
    {remove_file, "Remove file: '{}'"},
    {create_directory, "Create directory: '{}'"},
    {remove_directory, "Remove directory: '{}'"},
    {undo_before_execute, "'{0}::undo' called before successful '{0}::execute'"}
  };
};


} // namespace dropclone::messagecode