#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <dropclone/clone_config.hpp>
#include <dropclone/exception.hpp>
#include <dropclone/errorcode.hpp>
#include <dropclone/utility.hpp>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;
namespace dc = dropclone;

static fs::path const test_dir_path = fs::current_path() / fs::path{"../../test/"};
static fs::path const log_path = test_dir_path / fs::path{"log"};
static fs::path const config_path = test_dir_path / fs::path{"config/dropclone.json"};

TEST_CASE("sanitize resolves relative log_directory based on config path", "[clone_config][sanitize]") { 
  fs::path const log_directory_relative{"log/rel"};
  fs::path const config_log_path = config_path.parent_path() / log_directory_relative;
  dc::clone_config config{};
  config.log_directory = log_directory_relative;
  config.sanitize(config_path.lexically_normal());
  REQUIRE_THAT(config.log_directory.string(), 
    Catch::Matchers::Equals(config_log_path.lexically_normal().string()));
}

TEST_CASE("sanitize assigns default log_directory based on config path if empty", "[clone_config][sanitize]") { 
  fs::path const config_log_path = config_path.parent_path() / fs::path{"log"};
  dc::clone_config config{};
  config.log_directory = "";
  config.sanitize(config_path.lexically_normal());
  REQUIRE_THAT(config.log_directory.string(), 
    Catch::Matchers::Equals(config_log_path.lexically_normal().string()));
}

TEST_CASE("sanitize throws if log_directory cannot be created", "[clone_config][sanitize]") { 
  fs::path readonly_directory = log_path / fs::path{"readonly"};
  fs::perms original_permissions{};

  try {
    fs::create_directories(readonly_directory);
    original_permissions = fs::status(readonly_directory).permissions();
    fs::permissions(readonly_directory, 
      fs::perms::owner_write | fs::perms::group_write | fs::perms::others_write, 
      fs::perm_options::remove
    );
  } catch(fs::filesystem_error const& e) {
    fs::permissions(readonly_directory, original_permissions, fs::perm_options::replace);
    FAIL(dc::utility::formatter<dc::errorcode::test>::format(
      dc::errorcode::test::failed_prepare_readonly_dir,
      readonly_directory.lexically_normal().string(),
      e.what()
    ));
  }

  dc::clone_config config{};
  config.log_directory = readonly_directory / fs::path{"fails"};

  using Catch::Matchers::ContainsSubstring;
  using Catch::Matchers::MessageMatches;
  REQUIRE_THROWS_MATCHES(config.sanitize(config_path), dc::exception, 
    MessageMatches(ContainsSubstring("filesystem_error.001")));
}

TEST_CASE("sanitize normalizes absolute log_directory", "[clone_config][sanitize]") { 
  dc::clone_config config{};
  config.log_directory = log_path;
  config.sanitize(config_path);
  REQUIRE_THAT(config.log_directory.string(), 
    Catch::Matchers::Equals(log_path.lexically_normal().string()));
}

TEST_CASE("sanitize stores config_path normalized", "[clone_config][sanitize]") {
  dc::clone_config config{};
  config.sanitize(config_path);
  REQUIRE_THAT(config.config_path.string(), 
    Catch::Matchers::Equals(config_path.lexically_normal().string()));
}