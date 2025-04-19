#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <dropclone/clone_config.hpp>
#include <dropclone/exception.hpp>
#include <filesystem>
#include <string>

#include <iostream>

namespace fs = std::filesystem;
namespace dc = dropclone;

static fs::path const test_dir_path = fs::current_path() / fs::path{"../../test/"};
static fs::path const log_path = test_dir_path / fs::path{"log"};
static fs::path const config_path = test_dir_path / fs::path{"config/dropclone.json"};

TEST_CASE("sanitize resolves relative log_directory based on config path", "[clone_config]") { 
  fs::path const log_directory_relative{"log/rel"};
  fs::path const config_log_path = config_path.parent_path() / log_directory_relative;
  dc::clone_config config{};
  config.log_directory = log_directory_relative;
  config.sanitize(config_path.lexically_normal());
  REQUIRE_THAT(config.log_directory.string(), 
    Catch::Matchers::Equals(config_log_path.lexically_normal().string()));
}

TEST_CASE("sanitize assigns default log_directory based on config path if empty", "[clone_config]") { 
  fs::path const config_log_path = config_path.parent_path() / fs::path{"log"};
  dc::clone_config config{};
  config.log_directory = "";
  config.sanitize(config_path.lexically_normal());
  REQUIRE_THAT(config.log_directory.string(), 
    Catch::Matchers::Equals(config_log_path.lexically_normal().string()));
}

TEST_CASE("sanitize throws if log_directory cannot be created", "[clone_config]") { 
  dc::clone_config config{};
  config.log_directory = log_path / fs::path{"readonly/fails"};
  using Catch::Matchers::ContainsSubstring;
  using Catch::Matchers::MessageMatches;
  REQUIRE_THROWS_MATCHES(config.sanitize(config_path), dc::exception, 
    MessageMatches(ContainsSubstring("filesystem_error.001")));
}

TEST_CASE("sanitize normalizes absolute log_directory", "[clone_config]") { 
  dc::clone_config config{};
  config.log_directory = log_path;
  config.sanitize(config_path);
  REQUIRE_THAT(config.log_directory.string(), 
    Catch::Matchers::Equals(log_path.lexically_normal().string()));
}