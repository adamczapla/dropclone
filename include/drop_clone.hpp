#pragma once

// #include <clone_manager.hpp>
#include <clone_config.hpp>
#include <filesystem>

namespace dropclone {

namespace fs = std::filesystem;

class drop_clone {
 public:
  drop_clone(fs::path config_path, config_parser) noexcept;

 private:
  auto init_startup_logger() -> void;
  auto init_task_logger() -> void;
  auto init_daemon_logger() -> void;

  fs::path config_path_; // maybe i'll remove it later
  config_parser parser_; // maybe i'll remove it later
  clone_config clone_config_;
};
  
} // namespace dropclone

