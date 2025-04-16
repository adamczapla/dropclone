#pragma once

#include <unordered_map>
#include <string_view>

namespace dropclone::messagecode {

namespace logger {
  
struct startup {
  static constexpr auto config_file_parsed = "startup_message.001";
  static constexpr auto config_validated    = "startup_message.002";
  static constexpr auto logging_ready       = "startup_message.003";

  static inline std::unordered_map<std::string_view, std::string_view> const messages{
    {config_file_parsed, "Configuration file '{}' successfully parsed"},
    {config_validated, "Configuration validated successfully – {} clone entries ready"},
    {logging_ready, "Logging initialized to: {}"}
  };
};

struct task {
  /** message codes */
  static inline std::unordered_map<std::string_view, std::string_view> const messages{};
};

struct daemon {
  /** message codes */
  static inline std::unordered_map<std::string_view, std::string_view> const messages{};
};

} // namespace logger

} // namespace dropclone::messagecode