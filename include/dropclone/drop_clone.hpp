#pragma once

#include <dropclone/clone_config.hpp>
#include <filesystem>

namespace dropclone {

namespace fs = std::filesystem;

class drop_clone {
 public:
  drop_clone(fs::path config_path, config_parser);

 private:
  auto init_config_logger() -> void;
  auto init_startup_logger() -> void;
  auto init_daemon_logger() -> void;

  fs::path config_path_; // maybe i'll remove it later
  config_parser parser_; // maybe i'll remove it later
  clone_config clone_config_;
};
  
} // namespace dropclone

