#include <clone_config.hpp>
#include <logger_manager.hpp>
#include <exception.hpp>
#include <errorcode.hpp>
#include <utility.hpp>
#include <string>
#include <string_view>
#include <optional>
#include <filesystem>
#include <unordered_map>
#include <ostream>

#include <iostream>

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

auto clone_config::sanitize(fs::path const& config_path) -> void {
  log_directory = log_directory.lexically_normal(); 
  try { 
    if (!log_directory.is_absolute()) { 
      log_directory = config_path.parent_path() /
        (log_directory.empty() ? fs::path{"log"} : log_directory);

      logger.get(logger_id::config)->warn(
        utility::formatter<errorcode::config>::format(
          errorcode::config::path_not_configured,
          "log_directory", log_directory.string()
      ));
    }
    fs::create_directories(log_directory);
  } catch (fs::filesystem_error const& e) {
    throw_exception<errorcode::filesystem>(
      errorcode::filesystem::could_not_create_directory,
      log_directory.string(),
      e.what()
    );
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