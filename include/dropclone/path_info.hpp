#pragma once

#include <dropclone/messagecode.hpp>
#include <dropclone/utility.hpp>
#include <filesystem>
#include <unordered_map>
#include <string>

namespace dropclone {

namespace fs = std::filesystem;

enum class path_conflict_t{none, size_mismatch, permission_mismatch, permission_denied};

namespace messagecode {

struct path_conflict {
  static constexpr auto size_mismatch         = "path_conflict_message.001";
  static constexpr auto permission_mismatch   = "path_conflict_message.002";

  static inline std::unordered_map<path_conflict_t, std::string_view> const messages{
    {path_conflict_t::size_mismatch, "file sizes differ for '{}' despite identical timestamps"},
    {path_conflict_t::permission_mismatch, "file permissions differ for '{}'"}
  };
};
  
} // namespace messagecode

namespace utility {

inline auto to_string(path_conflict_t conflict) -> std::string {
  namespace msc = messagecode;
  switch (conflict) {
    case path_conflict_t::none: 
      return std::string{"no conflict"};
    case path_conflict_t::size_mismatch:
      return std::string{msc::path_conflict::size_mismatch};
    case path_conflict_t::permission_mismatch:
      return std::string{msc::path_conflict::permission_mismatch};
  }
  return "unknown conflict";
}

} // namespace utility
  
  
struct path_info {
  enum class status {unchanged, added, updated};

  fs::file_time_type last_write_time{};
  uintmax_t file_size{};
  fs::perms file_perms{};
  status path_status{status::unchanged};
  path_conflict_t conflict{path_conflict_t::none};
  
  auto operator==(path_info const&) const -> bool = default;
};
  
} // namespace dropclone
  
namespace std {
  
template <> struct hash<dropclone::path_info> {
  auto operator()(dropclone::path_info const& info) const noexcept -> size_t {
    size_t seed{0};
    auto hasher = [&](auto const& hash_value) -> size_t {
      return hash_value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    };
    seed ^= hasher(hash<uintmax_t>{}(info.file_size));
    seed ^= hasher(hash<filesystem::perms>{}(info.file_perms));
    seed ^= hasher(hash<int64_t>{}(info.last_write_time.time_since_epoch().count()));
    return seed;
  }
};
  
} // namespace std