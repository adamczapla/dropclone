#pragma once

#include <path_info.hpp>
#include <filesystem>
#include <unordered_map>

namespace dropclone {

namespace fs = std::filesystem;

class clone_manager {
 public:
  using path_snapshot = std::unordered_map<fs::path, path_info>;

  explicit clone_manager(fs::path root);

  auto hash(path_snapshot const&) noexcept -> size_t;
  inline auto current() const noexcept -> path_snapshot const&;

 private:
  path_snapshot snapshot_{};
  fs::path root_;

  auto make_snapshot() -> void;
};

auto clone_manager::current() const noexcept -> path_snapshot const& {
  return snapshot_;
}

} // namespace dropclone