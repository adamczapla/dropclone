#pragma once

#include <string>
#include <string_view>
#include <format>

namespace dropclone::utility {

inline auto to_string(std::string_view code) -> std::string { return std::string{code}; }

template <typename message_type>
struct formatter {
  static auto format(auto code, auto const&... args) -> std::string {
    try {
      return std::vformat(message_type::messages.at(code), std::make_format_args(args...));
    } catch (std::format_error const& e) {
      return std::format("[format_error: {} ] for code {}", e.what(), utility::to_string(code));
    } catch (std::out_of_range const&) {
      return std::format("[missing_message_code: {}]", utility::to_string(code));
    }
  }
};

} // namespace dropclone 
