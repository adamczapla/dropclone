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
  fs::path config_path_; // maybe i'll remove it later
  config_parser parser_; // maybe i'll remove it later
  clone_config clone_config_;
};
  
} // namespace dropclone

