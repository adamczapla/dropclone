#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

namespace dropclone::errorcode {

namespace config { 
  inline constexpr auto file_not_found            = "config_error.001";
  inline constexpr auto parse_error               = "config_error.002";
  inline constexpr auto conversion_error          = "config_error.003";
  inline constexpr auto path_not_absolute         = "config_error.004";
  inline constexpr auto invalid_clone_mode        = "config_error.005";
  inline constexpr auto overlapping_path_conflict = "config_error.005";

} // namespace dropclone::errorcode::config

namespace filesystem {} // namespace dropclone::errorcode::filesystem
/* namespace ... */

} // namespace dropclone::errorcode

namespace dropclone {

class exception : public std::runtime_error {
 public:
  exception(std::string_view error_code, std::string_view error_message)
    : std::runtime_error{format(error_code, error_message)}
  {}

  inline auto format(std::string_view error_code, std::string_view error_message) -> std::string;
};

auto exception::format(std::string_view error_code, std::string_view error_message) -> std::string {
  return std::string{"[dropclone.exception."}.append(error_code).append("] ").append(error_message);
}

} // namespace dropclone

