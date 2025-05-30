#pragma once

#include <nlohmann/json.hpp>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include <regex>

namespace dropclone {

namespace fs = std::filesystem;

enum class clone_mode { copy, move, undefined };

NLOHMANN_JSON_SERIALIZE_ENUM(clone_mode, {
  {clone_mode::undefined, "undefined"},
  {clone_mode::copy, "copy"},
  {clone_mode::move, "move"}
})

struct config_entry {
  using patterns_type = std::vector<std::regex>;
  using raw_patterns_type = std::vector<std::string>;

  config_entry(fs::path source_directory, fs::path destination_directory);
  config_entry(fs::path source_directory, fs::path destination_directory, clone_mode mode);
  config_entry(fs::path source_directory, fs::path destination_directory, clone_mode mode, 
               raw_patterns_type& exclude_patterns, raw_patterns_type& include_patterns);

  fs::path source_directory{};
  fs::path destination_directory{};
  clone_mode mode{};
  patterns_type exclude_patterns{};
  patterns_type include_patterns{};

  // bidirectional_sync {true, false} // comming soon
  auto sanitize() -> void;
  auto filter(fs::path const&) -> bool;

  static auto compile_patterns(raw_patterns_type& raw_patterns) -> patterns_type;
};

class clone_config {
 public:
  std::vector<config_entry> entries{};
  fs::path config_path{};
  fs::path log_directory{};

  auto sanitize(fs::path const&) -> void;
  auto validate() -> void; 

 private:
  struct path_node {
    std::unordered_map<std::string, path_node> children{};
    bool is_terminal{false};
  };
  auto has_conflict(path_node& root_node, fs::path const& path) const -> bool;
};

using config_parser = std::function<clone_config(fs::path const&)>;
  
} // namespace dropclone
