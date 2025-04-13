#include <clone_config.hpp>
#include <string>
#include <string_view>
#include <optional>
#include <filesystem>
#include <unordered_map>
#include <ostream>


namespace dropclone {

namespace fs = std::filesystem;
namespace rng = std::ranges;

auto config_entry::sanitize() -> std::optional<std::string_view> { 
  source_directory = source_directory.lexically_normal();
  destination_directory = destination_directory.lexically_normal();

  if (!source_directory.is_absolute()) {
    // this has to be an exception
    return "source_directory must be an absolute path";
  }

  if (!destination_directory.is_absolute()) {
    // this has to be an exception
    return "destination_directory must be an absolute path";
  }

  // if (mode == clone_mode::undefined) {}

  return {}; 
}

auto clone_config::has_conflict(path_node& root_node, fs::path const& path) const -> bool {
  path_node* current_node = &root_node;

  for (auto const& directory : path.lexically_normal()) {
      if (directory.empty()) { break; }
      if (current_node->is_terminal) { return true; }
      auto [child, _] = current_node->children.try_emplace(directory.string());
      current_node = &child->second;
  }

  if (current_node->is_terminal || !rng::empty(current_node->children)) { 
      return true; 
  }
  
  current_node->is_terminal = true;

  return false;
}

auto clone_config::validate() const -> std::optional<std::string> {
  path_node root_source{};
  path_node root_destination{};

  for (auto const& entry : entries) {
    if (has_conflict(root_source, entry.source_directory)) {
      return "Conflict detected in source_directory path: \"" + entry.source_directory.string() + "\""; 
    }
    if (has_conflict(root_destination, entry.destination_directory)) {
      return "Conflict detected in destination_directory path: \"" + entry.destination_directory.string() + "\"";
    }
  }

  return {};
}


auto operator<<(std::ostream& ostrm, clone_mode const& mode) -> std::ostream& {
  switch (mode) {
    case clone_mode::copy: ostrm << "copy"; break;
    case clone_mode::move: ostrm << "move"; break;
    case clone_mode::undefined: ostrm << "undefined"; break;
  }
  return ostrm;
}
  
} // namespace dropclone