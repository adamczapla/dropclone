#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <dropclone/nlohmann_json_parser.hpp>
#include <dropclone/exception.hpp>
#include <dropclone/errorcode.hpp>
#include <dropclone/utility.hpp>
#include <fstream>

#include <string>
#include <filesystem>

namespace fs = std::filesystem;
namespace dc = dropclone;

TEST_CASE("parser throws if could not open json file", "[nlohmann_json_parser]") {
  REQUIRE_THROWS_MATCHES(dc::nlohmann_json_parser{}("not_existing_path"), dc::exception, 
    Catch::Matchers::MessageMatches(Catch::Matchers::ContainsSubstring("config_error.001")));
}

TEST_CASE("parser throws if json file contains syntax error", "[nlohmann_json_parser]") {
  constexpr auto json_config = 
  R"(
  {
    "clone_config" : [ // comment causes syntax error
    {
      "source_directory" : "/home/source",
      "destination_directory" : "/home/destination/",
      "mode" : "copy", 
      "recursive" : true
    },
    "log_directory" : "/github/dropclone/log/"
  })";

  fs::path const config_path = fs::temp_directory_path() / fs::path{"dropclone_config_temp.json"};
  std::ofstream json_ofstrm{config_path, std::ios::trunc};
  if (!json_ofstrm.is_open()) { 
    FAIL(dc::utility::formatter<dc::errorcode::test>::format(
      dc::errorcode::test::could_not_open_temporary_file,
      config_path.lexically_normal().string()
    ));
  }
  json_ofstrm << json_config;

  REQUIRE_THROWS_MATCHES(dc::nlohmann_json_parser{}(config_path), dc::exception, 
    Catch::Matchers::MessageMatches(Catch::Matchers::ContainsSubstring("config_error.002")));
}