#include <dropclone/clone_transaction.hpp>
#include <dropclone/path_snapshot.hpp>
#include <dropclone/exception.hpp>
#include <filesystem>
#include <variant>
#include <stdexcept>
#include <algorithm>

#include <iostream> // remove it later

namespace dropclone {

namespace rng = std::ranges;
namespace fs = std::filesystem;

auto copy_command::execute() const -> void {
  try {
    for (auto const& entry : snapshot_.directories()) {
      std::cout << "copy_command: crate directory " << (destination_root_ / entry.first) 
                << '\n';
    }
    for (auto const& entry : snapshot_.files()) {
      std::cout << "copy_command: copy from " << (snapshot_.root() / entry.first) 
                << "to " << (destination_root_ / entry.first) 
                << '\n';
    }
  } catch (fs::filesystem_error const& err) {
    throw_exception<errorcode::filesystem>(
      errorcode::filesystem::copy_command_failed,
      err.path1().string(),
      err.path2().string(),
      err.what()
    );
  }
}
auto copy_command::undo() const -> void {}

auto rename_command::execute() const -> void {
  for (auto const& entry : snapshot_.directories()) {
    std::cout << "rename_command: crate directory " << (destination_root_ / entry.first) 
              << '\n';
  }
  for (auto const& entry : snapshot_.files()) {
    std::cout << "rename_command: rename from " << (snapshot_.root() / entry.first) 
              << " to " << destination_root_ / entry.first 
              << '\n';
  }
}
auto rename_command::undo() const -> void {}

auto remove_command::execute() const -> void {
  for (auto const& entry : snapshot_.files()) {
    std::cout << "remove_command: remove " << (snapshot_.root() / entry.first) 
              << '\n';
  }
  for (auto const& entry : snapshot_.directories()) {
    std::cout << "remove_command: remove directory " << (snapshot_.root() / entry.first) 
              << '\n';
  }
}
auto remove_command::undo() const -> void {}

auto clone_transaction::start() -> void {
  if (commands_.empty()) { return; }
  rng::for_each(commands_, [&](auto const& command) {
    try {
      std::visit([](auto const& cmd) { cmd.execute(); }, command);
      processed_commands_.push(command);
    } catch (std::exception const& err) {
      std::visit([](auto const& cmd) { cmd.undo(); }, command);
      try {
        rollback();
      } catch (std::exception const& rollback_err) {
        throw_exception<errorcode::transaction>(
          errorcode::transaction::rollback_failed,
          rollback_err.what()
        );
      }
      throw_exception<errorcode::transaction>(
        errorcode::transaction::start_failed,
        err.what()
      );
    }
  });
}

auto clone_transaction::rollback() -> void {
  while (!processed_commands_.empty()) {
    auto command = processed_commands_.top();
    std::visit([](auto const& cmd) { cmd.undo(); }, command);
    processed_commands_.pop();
  }
}

} // namespace dropclone  