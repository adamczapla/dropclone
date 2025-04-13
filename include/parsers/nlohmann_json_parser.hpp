#pragma once

#include <clone_config.hpp>
#include <nlohmann/json.hpp> 
#include <filesystem>

namespace dropclone {

namespace fs = std::filesystem;

struct nlohmann_json_parser {
  auto operator()(fs::path) -> clone_config;
};
  
} // namespace dropclone