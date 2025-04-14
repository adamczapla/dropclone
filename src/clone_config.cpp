#include <clone_config.hpp>
#include <exception.hpp>
#include <errorcode.hpp>
#include <string>
#include <string_view>
#include <optional>
#include <filesystem>
#include <unordered_map>
#include <ostream>

namespace dropclone {

namespace fs = std::filesystem;
namespace rng = std::ranges;

auto config_entry::sanitize() -> void { 
  if (!source_directory.is_absolute()) {
    throw_exception<errorcode::config>(
      errorcode::config::path_not_absolute, "source_directory"
    );
  }

  if (!destination_directory.is_absolute()) {
    throw_exception<errorcode::config>(
      errorcode::config::path_not_absolute, "destination_directory"
    );
  }

  if (mode == clone_mode::undefined) {
    throw_exception<errorcode::config>(
      errorcode::config::invalid_clone_mode, "mode"
    );
  }

  source_directory = source_directory.lexically_normal();
  destination_directory = destination_directory.lexically_normal();
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

auto clone_config::validate() const -> void {
  path_node root_source{};
  path_node root_destination{};

  for (auto const& entry : entries) {
    if (has_conflict(root_source, entry.source_directory)) {
      throw_exception<errorcode::config>(
        errorcode::config::overlapping_path_conflict, 
        "source_directory", entry.source_directory.string()
      );
    }

    if (has_conflict(root_destination, entry.destination_directory)) {
      throw_exception<errorcode::config>(
        errorcode::config::overlapping_path_conflict, 
        "destination_directory", entry.destination_directory.string()
      );
    }
  }
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