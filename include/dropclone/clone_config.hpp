#pragma once

#include <nlohmann/json.hpp>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include <unordered_map>
#include <ostream>

namespace dropclone {

namespace fs = std::filesystem;

enum class clone_mode { copy, move, undefined };

auto operator<<(std::ostream&, clone_mode const&) -> std::ostream&;

NLOHMANN_JSON_SERIALIZE_ENUM(clone_mode, {
  {clone_mode::undefined, "undefined"},
  {clone_mode::copy, "copy"},
  {clone_mode::move, "move"}
})

struct config_entry {
  fs::path source_directory{};
  fs::path destination_directory{};
  clone_mode mode{};
  bool recursive{true};
  // + exclude_patterns as regular expression
  auto sanitize() -> void;
};

class clone_config {
 public:
  std::vector<config_entry> entries{};
  fs::path log_directory{};

  auto validate() const -> void; 
  auto sanitize(fs::path const&) -> void;

 private:
  struct path_node {
    std::unordered_map<std::string, path_node> children{};
    bool is_terminal{false};
  };
  auto has_conflict(path_node& root_node, fs::path const& path) const -> bool;
};

using config_parser = std::function<clone_config(fs::path const&)>;
  
} // namespace dropclone
