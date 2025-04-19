#pragma once

#include <unordered_map>
#include <string_view>

namespace dropclone::messagecode {

namespace logger {
  
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

struct startup {
  /** message codes */
  static inline std::unordered_map<std::string_view, std::string_view> const messages{};
};

struct daemon {
  /** message codes */
  static inline std::unordered_map<std::string_view, std::string_view> const messages{};
};

} // namespace logger

} // namespace dropclone::messagecode