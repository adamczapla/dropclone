#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <dropclone/clone_config.hpp>
#include <dropclone/exception.hpp>
#include <filesystem>
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