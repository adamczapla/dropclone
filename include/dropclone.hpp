#pragma once

#include <filesystem>
#include <unordered_map>

namespace dropclone {

namespace fs = std::filesystem;

struct file_info {
  fs::file_time_type last_write_time{};
  uintmax_t file_size{};
  fs::perms file_perms{};

  auto operator==(file_info const&) const -> bool = default;
};

} // dropclone

namespace std {

template <> struct hash<dropclone::file_info> {
  auto operator()(dropclone::file_info const& info) const noexcept -> size_t {
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

} // std