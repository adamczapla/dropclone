#pragma once

#include <dropclone/path_info.hpp>
#include <unordered_map>
#include <map>
#include <filesystem>
#include <functional>
#include <unordered_set> 
#include <set>

namespace dropclone {

  namespace fs = std::filesystem;
  
  class path_snapshot {
   public:
    using snapshot_entries = std::unordered_map<fs::path, path_info>; 
    using snapshot_directories = std::map<fs::path, path_info>;
    using uncertain_processing_paths = std::unordered_set<fs::path>;
    using path_filter = std::function<bool(fs::path const&)>;
    using entry_filter = std::function<bool(snapshot_entries::value_type const&)>;
  
    explicit path_snapshot(fs::path root);
  
    auto make(path_filter filter = {}) -> void;
    auto operator-(path_snapshot const& other) -> path_snapshot;

    inline auto root() const noexcept -> fs::path;
    inline auto hash() const noexcept -> size_t;
    inline auto entries() const noexcept -> snapshot_entries const&;
    inline auto files() const noexcept -> snapshot_entries const&;
    inline auto directories() const noexcept -> snapshot_directories const&;

    inline auto files() noexcept -> snapshot_entries&;
    inline auto directories() noexcept -> snapshot_directories&;

    auto add_files(snapshot_entries const& files, entry_filter filter) -> void;
    auto add_directories(snapshot_directories const& directories, entry_filter filter) -> void;

    auto rebase(fs::path const& new_root) -> void;
  
   private:
    fs::path root_;
    snapshot_entries entries_{};
    snapshot_entries conflicts_{};
    snapshot_entries files_{};
    snapshot_directories directories_{};
    uncertain_processing_paths uncertain_processing_paths_{};
    size_t hash_{};
  
    auto compute_hash() const -> size_t;
  };

  auto path_snapshot::root() const noexcept -> fs::path { return root_; }
  auto path_snapshot::hash() const noexcept -> size_t { return hash_; }
  auto path_snapshot::entries() const noexcept -> snapshot_entries const& { return entries_; }

  auto path_snapshot::files() const noexcept -> snapshot_entries const& { return files_; }
  auto path_snapshot::files() noexcept -> snapshot_entries& { return files_; }

  auto path_snapshot::directories() const noexcept -> snapshot_directories const& { return directories_; }
  auto path_snapshot::directories() noexcept -> snapshot_directories& { return directories_; }
} // namespace dropclone
