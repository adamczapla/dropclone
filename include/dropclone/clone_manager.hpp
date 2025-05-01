#pragma once

#include <dropclone/clone_config.hpp>
#include <dropclone/path_snapshot.hpp>

namespace dropclone {

namespace fs = std::filesystem;

class clone_manager {
 public:
  clone_manager(config_entry entry);

  auto sync() -> void;
  auto copy(path_snapshot const& source_snapshot, fs::path const& destination_root) -> void;
  auto remove(path_snapshot const& snapshot) -> void;

 private:
  path_snapshot source_snapshot_;
  path_snapshot destination_snapshot_;
  config_entry entry_;
};

} // namespace dropclone