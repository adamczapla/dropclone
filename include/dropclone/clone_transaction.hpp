#pragma once

#include <dropclone/path_snapshot.hpp>
#include <concepts>
#include <variant>
#include <stack>
#include <vector> 
#include <utility>
#include <string_view>
#include <cstdint>
#include <functional>

namespace dropclone {

namespace fs = std::filesystem;

template <typename command_type>
concept is_clone_command = requires (command_type command) {
  {command.execute()} -> std::same_as<void>;
  {command.undo()} -> std::same_as<void>;
};

enum class command_status {uninitialized, success, failure};

class clone_transaction;

class command_base {
 protected:
  command_base(path_snapshot snapshot) : snapshot_{std::move(snapshot)} {}

  auto execute(std::string_view command_name, std::string_view errorcode,
               std::function<void(void)> execute) -> void;
  auto undo(std::string_view command_name, std::string_view errorcode,
               std::function<void(void)> undo) -> void;

  path_snapshot snapshot_;
  command_status execute_status_{command_status::uninitialized};
  command_status undo_status_{command_status::uninitialized};

  inline auto has_data() const noexcept -> bool;

  friend class clone_transaction;
};

auto command_base::has_data() const noexcept -> bool { return snapshot_.has_data(); }

enum class behavior_policies { none, duplicate };

class copy_command : public command_base {
 public:
  copy_command(path_snapshot snapshot, fs::path destination_root, behavior_policies behavior_policy = {}) 
    : command_base{std::move(snapshot)}, destination_root_{std::move(destination_root)},
      behavior_policy_{behavior_policy}
  {}

  auto execute() -> void;
  auto undo() -> void;

 private:
  fs::path destination_root_;
  behavior_policies behavior_policy_;
};

class rename_command : public command_base {
 public:
  rename_command(path_snapshot snapshot, fs::path destination_root) 
    : command_base{std::move(snapshot)}, 
      destination_root_{std::move(destination_root)} 
  {}

  auto execute() -> void;
  auto undo() -> void;

 private:
  fs::path destination_root_;
};

class remove_command : public command_base {
 public:
  remove_command(path_snapshot snapshot)
    : command_base{std::move(snapshot)} 
  {}

  auto execute() -> void;
  auto undo() -> void;
};

static_assert(is_clone_command<copy_command>);
static_assert(is_clone_command<rename_command>);
static_assert(is_clone_command<remove_command>);

using clone_command = std::variant<copy_command, rename_command, remove_command>;

class clone_transaction {
 public:
  inline auto add(clone_command command) -> void;
  auto start() -> void;

 private:
  std::vector<clone_command> commands_{};
  std::stack<clone_command> processed_commands_{};

  auto try_undo(clone_command command, std::uint8_t max_retries) -> void;
  auto log_unrecovered_entries() -> void;
  auto reset_command_statuses() -> void;
  auto rollback() -> void;
  auto reset() -> void;
};

auto clone_transaction::add(clone_command command) -> void { 
  commands_.push_back(std::move(command)); 
}

auto log_enter_command(std::string_view command_name, std::string_view function_name) -> void;
auto log_leave_command(std::string_view command_name, std::string_view function_name) -> void;

auto create_directory(fs::path const& directory_path) -> void;
auto remove_directory(fs::path const& directory_path) -> void;

auto create_directories(path_snapshot::snapshot_directories& directories, 
                        fs::path const& destination_root,
                        bool extract_on_success = false) -> void;

auto remove_directories(path_snapshot::snapshot_directories& directories,
                        fs::path const& source_root,
                        bool extract_on_success = false) -> void;
        
auto copy_files(path_snapshot::snapshot_entries& files, 
                fs::path const& source_root, 
                fs::path const& destination_root,
                bool extract_on_success = false, 
                fs::copy_options options = {}) -> void;

auto copy_duplicate(path_snapshot::snapshot_entries& files, 
                fs::path const& source_root, 
                fs::path const& destination_root) -> void;

auto rename_files(path_snapshot::snapshot_entries& files, 
                  fs::path const& source_root, 
                  fs::path const& destination_root,
                  bool extract_on_success = false) -> void;

auto remove_files(path_snapshot::snapshot_entries& files, 
                  fs::path const& source_root,
                  bool extract_on_success = false) -> void;

} // namespace dropclone

