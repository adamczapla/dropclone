#pragma once

#include <clone_manager.hpp>
#include <clone_config.hpp>

namespace dropclone {

class drop_clone {
 public:
  explicit drop_clone(config_parser) noexcept;

 private:
  config_parser parser_;
};
  
} // namespace dropclone

