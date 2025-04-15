#pragma once

#include <unordered_map>
#include <format>
#include <string_view>
#include <string>

namespace dropclone::errorcode {

struct config {
  static constexpr auto file_not_found            = "config_error.001";
  static constexpr auto parse_error               = "config_error.002";
  static constexpr auto conversion_error          = "config_error.003";
  static constexpr auto path_not_absolute         = "config_error.004";
  static constexpr auto invalid_clone_mode        = "config_error.005";
  static constexpr auto overlapping_path_conflict = "config_error.006";
  
  static inline std::unordered_map<std::string_view, std::string_view> const error_messages{
    {std::string_view{file_not_found}, "cannot open config file: {}\n"},
    {std::string_view{parse_error}, "could not parse config file {} |\n↳ origin error: \n\t↳ {}\n"},
    {std::string_view{conversion_error}, "conversion error |\n↳ origin error: \n\t↳ {}\n"},
    {std::string_view{path_not_absolute}, "'{}' must be an absolte path\n"},
    {std::string_view{invalid_clone_mode}, "'{}' must be (copy or move)\n"},
    {std::string_view{overlapping_path_conflict}, "Overlapping path detected in '{}': {}\n"}
  };
};

struct filesystem {
  static constexpr auto could_not_create_directory = "filesystem_error.001";
  
  static inline std::unordered_map<std::string_view, std::string_view> const error_messages{
    {std::string_view{could_not_create_directory}, "could not create directory: {} |\n↳ origin error: \n\t↳ {}\n"}
  };
};

struct logger {
  static constexpr auto logger_id_not_found = "logger_error.001";

  static inline std::unordered_map<std::string_view, std::string_view> const error_messages{
    {std::string_view{logger_id_not_found}, "logger_id '{}' not registered – using 'console' logger as fallback.\n"}
  };
};

template <typename error_type>
struct formatter {
  static std::string format(std::string_view code, auto const&... args) {
    try {
      return std::vformat(error_type::error_messages.at(code), 
                          std::make_format_args(args...));
    } catch (std::format_error const& e) {
      return std::string{"[format_error: "}
              .append(e.what())
              .append("] for code ")
              .append(code);
    } catch (std::out_of_range const&) {
      return std::string{"[missing_error_code: "}
              .append(code)
              .append("]");
    }
  }
};
  
} // namespace dropclone::errorcode