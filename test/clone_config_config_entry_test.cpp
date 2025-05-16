#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <catch2/matchers/catch_matchers_vector.hpp>

#include <catch2/matchers/catch_matchers_exception.hpp>
#include <dropclone/clone_config.hpp>
#include <dropclone/exception.hpp>
#include <dropclone/errorcode.hpp>
#include <dropclone/utility.hpp>
#include <filesystem>
#include <vector>
#include <string>

namespace fs = std::filesystem;
namespace dc = dropclone;

TEST_CASE("sanitize throws if source_directory is not absolute", "[clone_config][config_entry]") { 
  dc::config_entry entry{
    fs::path{"dropclone/test/"},
    fs::path{"/dropclone/test/"},
    dc::clone_mode::copy
  };
  REQUIRE_THROWS_AS(entry.sanitize(), dc::exception);
}

TEST_CASE("sanitize throws if source_directory is empty", "[clone_config][config_entry]") { 
  dc::config_entry entry{
    fs::path{""},
    fs::path{"/dropclone/test/"},
    dc::clone_mode::copy
  };
  REQUIRE_THROWS_AS(entry.sanitize(), dc::exception);
}

TEST_CASE("sanitize throws if destination_directory is not absolute", "[clone_config][config_entry]") { 
  dc::config_entry entry{
    fs::path{"/dropclone/test/"},
    fs::path{"dropclone/test/"},
    dc::clone_mode::copy
  };
  REQUIRE_THROWS_AS(entry.sanitize(), dc::exception);
}

TEST_CASE("sanitize throws if destination_directory is empty", "[clone_config][config_entry]") { 
  dc::config_entry entry{
    fs::path{"/dropclone/test/"},
    fs::path{""},
    dc::clone_mode::copy
  };
  REQUIRE_THROWS_AS(entry.sanitize(), dc::exception);
}

TEST_CASE("sanitize throws if mode is undefined", "[clone_config][config_entry]") { 
  dc::config_entry entry{
    fs::path{"/dropclone/test/"},
    fs::path{"/dropclone/test/"},
    dc::clone_mode::undefined
  };
  REQUIRE_THROWS_AS(entry.sanitize(), dc::exception);
}

TEST_CASE("sanitize normalizes paths if input is valid", "[clone_config][config_entry]") {
  dc::config_entry entry{
    fs::path{"/dropclone/test/"},
    fs::path{"/dropclone/test/"},
    dc::clone_mode::copy
  };
  REQUIRE_NOTHROW(entry.sanitize());
}

TEST_CASE("sanitize normalizes source and destination paths", "[clone_config][config_entry]") {
  dc::config_entry entry{
    fs::path{"/./dropclone/../dropclone/test/"},
    fs::path{"/dropclone/./test/../test/"},
    dc::clone_mode::copy
  };
  entry.sanitize();

  using Catch::Matchers::Equals;
  CHECK_THAT(entry.source_directory.string(), Equals("/dropclone/test/"));
  CHECK_THAT(entry.destination_directory.string(), Equals("/dropclone/test/"));
}

static fs::path const path_root{"/"};
static fs::path const path_a{"/a"};
static fs::path const path_a_slash{"/a/"};
static fs::path const path_b{"/b"};
static fs::path const path_b_slash{"/b/"};
static fs::path const path_ab{"/a/b"};
static fs::path const path_ab_slash{"/a/b/"};

static fs::path const path_c{"/c"};
static fs::path const path_d{"/d"};

using Catch::Matchers::ContainsSubstring;
using Catch::Matchers::MessageMatches;

TEST_CASE("validate throws if source_directory conflicts with another source_directory", "[clone_config][validate]") {
  dc::clone_config config{};

  auto const message_matches = MessageMatches(ContainsSubstring("source_directory"));

  config.entries = {{path_a, path_b}, {path_a_slash, path_c}};
  CHECK_THROWS_MATCHES(config.validate(), dc::exception, message_matches);

  config.entries = {{path_a, path_b}, {path_ab, path_c}};
  CHECK_THROWS_MATCHES(config.validate(), dc::exception, message_matches);

  config.entries = {{path_a, path_b}, {path_ab_slash, path_c}};
  CHECK_THROWS_MATCHES(config.validate(), dc::exception, message_matches);

  config.entries = {{path_a, path_b}, {path_ab, path_c}};
  CHECK_THROWS_MATCHES(config.validate(), dc::exception, message_matches);

  config.entries = {{path_a_slash, path_b}, {path_a, path_c}};
  CHECK_THROWS_MATCHES(config.validate(), dc::exception, message_matches);

  config.entries = {{path_ab, path_b}, {path_a, path_c}};
  CHECK_THROWS_MATCHES(config.validate(), dc::exception, message_matches);

  config.entries = {{path_ab_slash, path_b}, {path_a, path_c}};
  CHECK_THROWS_MATCHES(config.validate(), dc::exception, message_matches);

  config.entries = {{path_ab, path_b}, {path_a, path_c}};
  CHECK_THROWS_MATCHES(config.validate(), dc::exception, message_matches);
}

TEST_CASE("validate throws if destination_directory conflicts with another destination_directory", "[clone_config][validate]") {
  dc::clone_config config{};

  auto const message_matches = MessageMatches(ContainsSubstring("destination_directory"));

  config.entries = {{path_b, path_a}, {path_c, path_a_slash}};
  CHECK_THROWS_MATCHES(config.validate(), dc::exception, message_matches);

  config.entries = {{path_b, path_a}, {path_c, path_ab}};
  CHECK_THROWS_MATCHES(config.validate(), dc::exception, message_matches);

  config.entries = {{path_b, path_a}, {path_c, path_ab_slash}};
  CHECK_THROWS_MATCHES(config.validate(), dc::exception, message_matches);

  config.entries = {{path_b, path_a}, {path_c, path_ab}};
  CHECK_THROWS_MATCHES(config.validate(), dc::exception, message_matches);

  config.entries = {{path_b, path_a_slash}, {path_c, path_a}};
  CHECK_THROWS_MATCHES(config.validate(), dc::exception, message_matches);

  config.entries = {{path_b, path_ab}, {path_c, path_a}};
  CHECK_THROWS_MATCHES(config.validate(), dc::exception, message_matches);

  config.entries = {{path_b, path_ab_slash}, {path_c, path_a}};
  CHECK_THROWS_MATCHES(config.validate(), dc::exception, message_matches);

  config.entries = {{path_b, path_ab}, {path_c, path_a}};
  CHECK_THROWS_MATCHES(config.validate(), dc::exception, message_matches);
}

TEST_CASE("validate throws if source_directory conflicts with a destination_directory", "[clone_config][validate]") {
  dc::clone_config config{};

  // source / destination same entry
  config.entries = {{path_root, path_a}, {path_b, path_c}};
  CHECK_THROWS_AS(config.validate(), dc::exception);

  config.entries = {{path_a, path_ab}, {path_b, path_c}};
  CHECK_THROWS_AS(config.validate(), dc::exception);

  config.entries = {{path_a, path_ab_slash}, {path_b, path_c}};
  CHECK_THROWS_AS(config.validate(), dc::exception);

  config.entries = {{path_a_slash, path_ab_slash}, {path_b, path_c}};
  CHECK_THROWS_AS(config.validate(), dc::exception);

  // source / destination different entries
  config.entries = {{path_root, path_b}, {path_c, path_a}};
  CHECK_THROWS_AS(config.validate(), dc::exception);

  config.entries = {{path_a, path_b}, {path_c, path_ab}};
  CHECK_THROWS_AS(config.validate(), dc::exception);

  config.entries = {{path_a, path_b}, {path_c, path_ab_slash}};
  CHECK_THROWS_AS(config.validate(), dc::exception);

  config.entries = {{path_a_slash, path_b}, {path_c, path_ab_slash}};
  CHECK_THROWS_AS(config.validate(), dc::exception);

  // destination / source different entries
  config.entries = {{path_b, path_root}, {path_a, path_c}};
  CHECK_THROWS_AS(config.validate(), dc::exception);

  config.entries = {{path_b, path_a}, {path_ab, path_c}};
  CHECK_THROWS_AS(config.validate(), dc::exception);

  config.entries = {{path_b, path_a}, {path_ab_slash, path_c}};
  CHECK_THROWS_AS(config.validate(), dc::exception);

  config.entries = {{path_b, path_a_slash}, {path_ab_slash, path_c}};
  CHECK_THROWS_AS(config.validate(), dc::exception);
}

TEST_CASE("validate passes if no source or destination directories conflict", "[clone_config][validate]") {
  dc::clone_config config{};
  
  config.entries = { {path_a, path_b}, {path_c, path_d}};
  CHECK_NOTHROW(config.validate());

  config.entries = { {path_ab, path_b}, {path_c, path_d}};
  CHECK_NOTHROW(config.validate());

  config.entries = { {path_a, path_c}, {path_b, path_d}};
  CHECK_NOTHROW(config.validate());

  config.entries = { {path_a_slash, path_c}, {path_b_slash, path_d}};
  CHECK_NOTHROW(config.validate());

  config.entries = { {path_ab_slash, path_c}, {path_d, path_b_slash}};
  CHECK_NOTHROW(config.validate());
}

TEST_CASE("validate throws if no entries defined in clone_config", "[clone_config][validate]") {
  dc::clone_config config{};

  REQUIRE_THROWS_MATCHES(config.validate(), dc::exception, 
    Catch::Matchers::MessageMatches(Catch::Matchers::ContainsSubstring("config_error.009")));
}

