#pragma once

#include <string>
#include <string_view>
#include <format>

namespace dropclone::utility {

template <typename message_type>
struct formatter {
  static auto format(std::string_view code, auto const&... args) -> std::string {
    try {
      return std::vformat(message_type::messages.at(code), std::make_format_args(args...));
    } catch (std::format_error const& e) {
      return std::format("[format_error: {} ] for code {}", e.what(), code);
    } catch (std::out_of_range const&) {
      return std::format("[missing_message_code: {}]", code);
    }
  }
};

} // namespace dropclone 
