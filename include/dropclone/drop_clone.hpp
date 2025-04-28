#pragma once

#include <dropclone/clone_config.hpp>
#include <dropclone/clone_manager.hpp>
#include <filesystem>
#include <vector>

namespace dropclone {

namespace fs = std::filesystem;

class drop_clone {
 public:
  drop_clone(fs::path config_path, config_parser);
  auto sync() -> void;

 private:
  auto init_config_logger() -> void;
  auto init_startup_logger() -> void;
  auto init_daemon_logger() -> void;

  clone_config clone_config_;
  std::vector<clone_manager> managers_{};
};
  
} // namespace dropclone

