#define JSON_DIAGNOSTICS 1

#include <nlohmann/json.hpp>
#include <parsers/nlohmann_json_parser.hpp>
#include <exception.hpp>
#include <clone_config.hpp>
#include <filesystem>
#include <fstream>
#include <cstdlib>

namespace dropclone {

namespace fs = std::filesystem;
using json = nlohmann::json;
  
auto nlohmann_json_parser::operator()(fs::path config_path) -> clone_config {
  std::ifstream istrm_config{config_path}; 

  if(!istrm_config.is_open()) { 
    throw dropclone::exception{
      errorcode::config::file_not_found,
      "cannot open config file: " + config_path.string() 
    };
  }

  json json_config{};

  try {
    json_config = json::parse(istrm_config);
  } catch (json::parse_error const& e) {
    throw dropclone::exception{
      errorcode::config::file_not_found,
      e.what()
    };
  }

  clone_config config{};

  try {
    for (auto const& elem : json_config["clone_config"]) {
      config.entries.emplace_back(
        elem["source_directory"],
        elem["destination_directory"],
        elem["mode"],
        elem["recursive"]
      );
    }
  } catch (json::exception const& e) {
    throw dropclone::exception{
      errorcode::config::conversion_error,
      e.what()
    };
  }

  return config;
}
  
} // namespace dropclone 