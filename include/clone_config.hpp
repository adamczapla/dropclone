#pragma once

#include <filesystem>
#include <functional>
#include <vector>

namespace dropclone {

namespace fs = std::filesystem;

struct config_entry { };

using clone_config = std::vector<config_entry>;
using config_parser = std::function<clone_config(fs::path const&)>;
  
} // namespace dropclone
