#pragma once

#include <dropclone/path_info.hpp>
#include <unordered_map>
#include <filesystem>
#include <functional>

namespace dropclone {

  namespace fs = std::filesystem;
  
  class path_snapshot {
   public:
    using snapshot_entries = std::unordered_map<fs::path, path_info>; 
    using path_filter = std::function<bool(fs::path const&)>;
  
    explicit path_snapshot(fs::path root);
  
    auto make(path_filter filter = {}) -> void;
    auto operator-(path_snapshot const& other) -> path_snapshot;

    inline auto root() const noexcept -> fs::path;
    inline auto hash() const noexcept -> size_t;
  
   private:
    fs::path root_;
    snapshot_entries entries_{};
    snapshot_entries conflicts_{};
    size_t hash_{};
  
    auto compute_hash() const -> size_t;
  };

  auto path_snapshot::root() const noexcept -> fs::path { return root_; }
  auto path_snapshot::hash() const noexcept -> size_t { return hash_; }

} // namespace dropclone
