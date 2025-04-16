#pragma once

#include <errorcode.hpp>
#include <utility.hpp>
#include <stdexcept>
#include <string_view>
#include <string>

namespace dropclone {

class exception : public std::runtime_error {
 public:
  exception(std::string_view error_code, std::string_view error_message)
    : std::runtime_error{format(error_code, error_message)}
  {}

 private:
  auto format(std::string_view error_code, std::string_view error_message) -> std::string {
    return std::string{"[dropclone.exception."}.append(error_code).append("] ").append(error_message);
  }
};

template <typename error_type, typename... Args>
auto throw_exception(std::string_view error_code, Args&&... args) {
  throw exception{
    error_code, utility::formatter<error_type>::format(error_code, std::forward<Args>(args)...)
  };
}

} // namespace dropclone

