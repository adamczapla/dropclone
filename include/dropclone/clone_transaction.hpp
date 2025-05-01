#pragma once

#include <dropclone/path_snapshot.hpp>
#include <concepts>
#include <variant>
#include <stack>
#include <vector> 
#include <utility>

namespace dropclone {

namespace fs = std::filesystem;

template <typename clone_command>
concept is_clone_command = requires (clone_command command) {
  {command.execute()} -> std::same_as<void>;
  {command.undo()} -> std::same_as<void>;
};

class copy_command {
 public:
  copy_command(path_snapshot snapshot, fs::path destination_root) 
    : snapshot_{std::move(snapshot)}, 
      destination_root_{std::move(destination_root)} 
  {}

  auto execute() const -> void;
  auto undo() const -> void;

 private:
  path_snapshot snapshot_;
  fs::path destination_root_;
};

class rename_command {
 public:
  rename_command(path_snapshot snapshot, fs::path destination_root) 
    : snapshot_{std::move(snapshot)}, 
      destination_root_{std::move(destination_root)} 
  {}

  auto execute() const -> void;
  auto undo() const -> void;

 private:
  path_snapshot snapshot_;
  fs::path destination_root_;
};

class remove_command {
 public:
  remove_command(path_snapshot snapshot)
    : snapshot_{std::move(snapshot)} 
  {}

  auto execute() const -> void;
  auto undo() const -> void;

 private:
  path_snapshot snapshot_;
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

  auto rollback() -> void;
};

auto clone_transaction::add(clone_command command) -> void { 
  commands_.push_back(command); 
}
  
} // namespace dropclone

