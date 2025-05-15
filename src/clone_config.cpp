#include <dropclone/clone_config.hpp>
#include <dropclone/logger_manager.hpp>
#include <dropclone/exception.hpp>
#include <dropclone/errorcode.hpp>
#include <dropclone/utility.hpp>
#include <string>
#include <string_view>
#include <filesystem>
#include <unordered_map>
#include <algorithm>
#include <utility>
#include <regex>
#include <ranges>

namespace dropclone {

namespace fs = std::filesystem;
namespace rng = std::ranges;

auto config_entry::compile_patterns(raw_patterns_type& raw_patterns) -> patterns_type {
  patterns_type result{};
  result.reserve(raw_patterns.size());

  rng::for_each(raw_patterns, [&](auto& pattern) {
    result.emplace_back(std::move(pattern), 
      std::regex::ECMAScript 
      | std::regex_constants::icase 
      | std::regex::optimize);
  });

  return result;
}

config_entry::config_entry(fs::path source_directory, fs::path destination_directory)
  : source_directory{std::move(source_directory)}, 
    destination_directory{std::move(destination_directory)}
{}

config_entry::config_entry(fs::path source_directory, fs::path destination_directory, clone_mode mode)
  : source_directory{std::move(source_directory)}, 
    destination_directory{std::move(destination_directory)}, 
    mode{mode}
{}

config_entry::config_entry(fs::path source_directory, fs::path destination_directory, 
                           clone_mode mode, raw_patterns_type& exclude_patterns, 
                           raw_patterns_type& include_patterns)
  : source_directory{std::move(source_directory)}, 
    destination_directory{std::move(destination_directory)},
    mode{mode}, exclude_patterns{std::move(compile_patterns(exclude_patterns))}, 
    include_patterns{std::move(compile_patterns(include_patterns))}
{}

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

  if (!exclude_patterns.empty() && !include_patterns.empty()) {
    throw_exception<errorcode::config>(
      errorcode::config::conflicting_fields, 
      "exclude", "include"
    );
  }

  source_directory = source_directory.lexically_normal();
  destination_directory = destination_directory.lexically_normal();
}

auto config_entry::filter(fs::path const& path) -> bool {
  auto absolute_path{path.string()};
  auto root_path{source_directory.string()};

  if (!absolute_path.starts_with(root_path)) { return false; }
  if (exclude_patterns.empty() && include_patterns.empty()) { 
    return true; 
  }

  auto const root_size{root_path.size()};
  auto relative_path = absolute_path.substr(
    root_size + 1, absolute_path.size() - root_size 
  );

  if (!exclude_patterns.empty()) {
    return !rng::any_of(exclude_patterns, [&](auto const& regex) {
      return std::regex_search(relative_path, regex);
    });
  }

  if (!include_patterns.empty()) {
    return rng::any_of(include_patterns, [&](auto const& regex) {
      return std::regex_search(relative_path, regex);
    });
  }

  return false; 
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

auto clone_config::validate() -> void {
  if (entries.empty()) {
    throw_exception<errorcode::config>(
      errorcode::config::no_entries_defined, config_path.string()
    );
  }

  path_node root_source{};
  path_node root_destination{};

  for (auto& entry : entries) {
    entry.sanitize();

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

auto clone_config::sanitize(fs::path const& path) -> void {
  config_path = path.lexically_normal();
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
  
} // namespace dropclone