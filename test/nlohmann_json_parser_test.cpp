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

static fs::path const temp_config_path = fs::temp_directory_path() / fs::path{"dropclone_config_temp.json"};

auto create_temporary_json_file(auto const& json_config) -> void {
  std::ofstream json_ofstrm{temp_config_path, std::ios::trunc};
  if (!json_ofstrm.is_open()) { 
    FAIL(dc::utility::formatter<dc::errorcode::test>::format(
      dc::errorcode::test::could_not_open_temporary_file,
      temp_config_path.lexically_normal().string()
    ));
  }
  json_ofstrm << json_config;
}

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
      }
    ],
    "log_directory" : "/github/dropclone/log/"
  })";

  create_temporary_json_file(json_config);

  REQUIRE_THROWS_MATCHES(dc::nlohmann_json_parser{}(temp_config_path), dc::exception, 
    Catch::Matchers::MessageMatches(Catch::Matchers::ContainsSubstring("config_error.002")));
}

TEST_CASE("parser throws if required field 'log_directory' is missing", "[nlohmann_json_parser]") {
  constexpr auto json_config = 
  R"(
  {
    "clone_config" : [
      {
        "source_directory" : "/home/adamc/",
        "destination_directory" : "/home/adamc/copy/",
        "mode" : "copy", 
        "recursive" : true
      }
    ] 
  })";

  create_temporary_json_file(json_config);

  REQUIRE_THROWS_MATCHES(dc::nlohmann_json_parser{}(temp_config_path), dc::exception, 
    Catch::Matchers::MessageMatches(Catch::Matchers::ContainsSubstring("config_error.008")));
}

TEST_CASE("parser throws if required field 'clone_config' is missing", "[nlohmann_json_parser]") {
  constexpr auto json_config = 
  R"(
  {
    "log_directory" : "/github/dropclone/log/"
  })";

  create_temporary_json_file(json_config);

  REQUIRE_THROWS_MATCHES(dc::nlohmann_json_parser{}(temp_config_path), dc::exception, 
    Catch::Matchers::MessageMatches(Catch::Matchers::ContainsSubstring("config_error.008")));
}

TEST_CASE("parser throws if required field 'source_directory' is missing", "[nlohmann_json_parser]") {
  constexpr auto json_config = 
  R"(
  {
    "clone_config" : [
      {
        "destination_directory" : "/home/destination/",
        "mode" : "copy", 
        "recursive" : true
      }
    ],
    "log_directory" : "/github/dropclone/log/"
  })";

  create_temporary_json_file(json_config);

  REQUIRE_THROWS_MATCHES(dc::nlohmann_json_parser{}(temp_config_path), dc::exception, 
    Catch::Matchers::MessageMatches(Catch::Matchers::ContainsSubstring("config_error.008")));
}

TEST_CASE("parser throws if required field 'destination_directory' is missing", "[nlohmann_json_parser]") {
  constexpr auto json_config = 
  R"(
  {
    "clone_config" : [ 
      {
        "source_directory" : "/home/source",
        "mode" : "copy", 
        "recursive" : true
      }
    ],
    "log_directory" : "/github/dropclone/log/"
  })";

  create_temporary_json_file(json_config);

  REQUIRE_THROWS_MATCHES(dc::nlohmann_json_parser{}(temp_config_path), dc::exception, 
    Catch::Matchers::MessageMatches(Catch::Matchers::ContainsSubstring("config_error.008")));
}

TEST_CASE("parser throws if required field 'mode' is missing", "[nlohmann_json_parser]") {
  constexpr auto json_config = 
  R"(
  {
    "clone_config" : [ 
      {
        "source_directory" : "/home/source",
        "destination_directory" : "/home/destination/",
        "recursive" : true
      }
    ],
    "log_directory" : "/github/dropclone/log/"
  })";

  create_temporary_json_file(json_config);

  REQUIRE_THROWS_MATCHES(dc::nlohmann_json_parser{}(temp_config_path), dc::exception, 
    Catch::Matchers::MessageMatches(Catch::Matchers::ContainsSubstring("config_error.008")));
}

TEST_CASE("parser passes if optional field 'recursive' is missing", "[nlohmann_json_parser]") {
  constexpr auto json_config = 
  R"(
  {
    "clone_config" : [ 
      {
        "source_directory" : "/home/source",
        "destination_directory" : "/home/destination/",
        "mode" : "copy"
      }
    ],
    "log_directory" : "/github/dropclone/log/"
  })";

  create_temporary_json_file(json_config);

  REQUIRE_NOTHROW(dc::nlohmann_json_parser{}(temp_config_path));
}

TEST_CASE("parser throws if field 'log_directory' has invalid type", "[nlohmann_json_parser]") {
  constexpr auto json_config = 
  R"(
  {
    "clone_config" : [
      {
        "source_directory" : "/home/source",
        "destination_directory" : "/home/destination/",
        "mode" : "copy", 
        "recursive" : true
      }
    ],
    "log_directory" : 0
  })";

  create_temporary_json_file(json_config);

  REQUIRE_THROWS_MATCHES(dc::nlohmann_json_parser{}(temp_config_path), dc::exception, 
    Catch::Matchers::MessageMatches(Catch::Matchers::ContainsSubstring("config_error.003")));
}

TEST_CASE("parser throws if field 'clone_config' has invalid type", "[nlohmann_json_parser]") {
  constexpr auto json_config = 
  R"(
  {
    "clone_config" : "wrong type", 
    "log_directory" : "/github/dropclone/log/"
  })";

  create_temporary_json_file(json_config);

  REQUIRE_THROWS_MATCHES(dc::nlohmann_json_parser{}(temp_config_path), dc::exception, 
    Catch::Matchers::MessageMatches(Catch::Matchers::ContainsSubstring("config_error.010")));
}

TEST_CASE("parser throws if field 'clone_config' is empty", "[nlohmann_json_parser]") {
  constexpr auto json_config = 
  R"(
  {
    "clone_config" : [], 
    "log_directory" : "/github/dropclone/log/"
  })";

  create_temporary_json_file(json_config);

  REQUIRE_THROWS_MATCHES(dc::nlohmann_json_parser{}(temp_config_path), dc::exception, 
    Catch::Matchers::MessageMatches(Catch::Matchers::ContainsSubstring("config_error.009")));
}

TEST_CASE("parser throws if field 'source_directory' has invalid type", "[nlohmann_json_parser]") {
  constexpr auto json_config = 
  R"(
  {
    "clone_config" : [
      {
        "source_directory" : [],
        "destination_directory" : "/home/destination/",
        "mode" : "copy", 
        "recursive" : true
      }
    ],
    "log_directory" : "/github/dropclone/log/"
  })";

  create_temporary_json_file(json_config);

  REQUIRE_THROWS_MATCHES(dc::nlohmann_json_parser{}(temp_config_path), dc::exception, 
    Catch::Matchers::MessageMatches(Catch::Matchers::ContainsSubstring("config_error.003")));
}

TEST_CASE("parser throws if field 'destination_directory' has invalid type", "[nlohmann_json_parser]") {
  constexpr auto json_config = 
  R"(
  {
    "clone_config" : [
      {
        "source_directory" : "/home/source",
        "destination_directory" : 0,
        "mode" : "copy",
        "recursive" : true
      }
    ],
    "log_directory" : "/github/dropclone/log/"
  })";

  create_temporary_json_file(json_config);

  REQUIRE_THROWS_MATCHES(dc::nlohmann_json_parser{}(temp_config_path), dc::exception, 
    Catch::Matchers::MessageMatches(Catch::Matchers::ContainsSubstring("config_error.003")));
}

TEST_CASE("parser throws if field 'recursive' has invalid type", "[nlohmann_json_parser]") {
  constexpr auto json_config = 
  R"(
  {
    "clone_config" : [
      {
        "source_directory" : "/home/source",
        "destination_directory" : "/home/destination/",
        "mode" : "copy",
        "recursive" : "true"
      }
    ],
    "log_directory" : "/github/dropclone/log/"
  })";

  create_temporary_json_file(json_config);

  REQUIRE_THROWS_MATCHES(dc::nlohmann_json_parser{}(temp_config_path), dc::exception, 
    Catch::Matchers::MessageMatches(Catch::Matchers::ContainsSubstring("config_error.003")));
}

TEST_CASE("parser passes if multiple entries are configured correctly", "[nlohmann_json_parser]") {
  constexpr auto json_config = 
  R"(
  {
    "clone_config" : [
      {
        "source_directory" : "/home/source",
        "destination_directory" : "/home/destination/",
        "mode" : "copy", 
        "recursive" : true
      },
      {
        "source_directory" : "/home/source2",
        "destination_directory" : "/home/destination2/",
        "mode" : "move", 
        "recursive" : false
      }
    ],
    "log_directory" : "/github/dropclone/log/"
  })";

  create_temporary_json_file(json_config);

  REQUIRE_NOTHROW(dc::nlohmann_json_parser{}(temp_config_path));
}