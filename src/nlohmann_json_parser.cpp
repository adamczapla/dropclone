#define JSON_DIAGNOSTICS 1

#include <nlohmann/json.hpp>
#include <dropclone/nlohmann_json_parser.hpp>
#include <dropclone/exception.hpp>
#include <dropclone/errorcode.hpp>
#include <dropclone/clone_config.hpp>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <string_view>
#include <string>

namespace dropclone {

namespace fs = std::filesystem;
using json = nlohmann::json;
  
auto nlohmann_json_parser::operator()(fs::path config_path) -> clone_config {
  std::ifstream istrm_config{config_path}; 

  if(!istrm_config.is_open()) { 
    throw_exception<errorcode::config>(
      errorcode::config::file_not_found, config_path.string()
    );
  }

  json json_config{};

  try {
    json_config = json::parse(istrm_config);
  } catch (json::parse_error const& e) {
    throw_exception<errorcode::config>(
      errorcode::config::parse_error, 
      config_path.string(),
      e.what()
    );
  }

  clone_config config{};

  try {
    auto throw_if_missing_required_field = [&](auto const& json, auto const& field) {
      if (!json.contains(field)) { 
        throw_exception<errorcode::config>(
          errorcode::config::missing_required_field, 
          field, config_path.string()
        );
      }
    };

    throw_if_missing_required_field(json_config, "log_directory");
    config.log_directory = json_config.value("log_directory", fs::path{});
    throw_if_missing_required_field(json_config, "clone_config");

    if (!json_config["clone_config"].is_array()) {
      throw_exception<errorcode::config>(
        errorcode::config::invalid_field_type, "clone_config"
      );
    }

    if (json_config["clone_config"].empty()) {
      throw_exception<errorcode::config>(
        errorcode::config::no_entries_defined, "clone_config"
      );
    }

    for (auto const& elem : json_config["clone_config"]) {
      throw_if_missing_required_field(elem, "source_directory");
      throw_if_missing_required_field(elem, "destination_directory");
      throw_if_missing_required_field(elem, "mode");

      if (elem.contains("exclude") && elem.contains("include")) {
        throw_exception<errorcode::config>(
          errorcode::config::conflicting_fields, 
          "exclude", "include"
        );
      }

      using patterns_t = std::vector<std::string>;

      auto const get_patterns = [&](std::string_view name) {
        if (!elem.contains(name)) { return patterns_t{}; }
        if (!elem[name].is_array()) {
          throw_exception<errorcode::config>(
            errorcode::config::invalid_field_type, name
          );
        }
        return elem[name].get<patterns_t>();
      };

      auto exclude_patterns = get_patterns("exclude");
      auto include_patterns = get_patterns("include");

      config.entries.emplace_back(
        elem["source_directory"],
        elem["destination_directory"],
        elem["mode"],
        exclude_patterns,
        include_patterns
      );
    }
  } catch (json::exception const& e) {
    throw_exception<errorcode::config>(
      errorcode::config::conversion_error, 
      e.what()
    );
  }

  return config;
}
  
} // namespace dropclone 