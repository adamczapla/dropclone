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

auto create_dirctories(path_snapshot::snapshot_directories const& directories, 
                       fs::path const& destination_root) -> void {
  rng::for_each(directories, [&](auto const& entry) {
    if(auto const directory = destination_root / entry.first; !fs::exists(directory)) {
      fs::create_directories(directory);
      std::cout << "crate directory " << directory << '\n';
    }
  });
}

auto remove_directories(path_snapshot::snapshot_directories const& directories,
                        fs::path const& source_root) -> void {
  rng::for_each(rng::crbegin(directories), rng::crend(directories),
    [&] (auto const& entry) { 
      auto const directory_path = source_root / entry.first;
      fs::remove(directory_path);
      std::cout << "remove directory " << directory_path << '\n';
  });
}

auto copy_command::execute() const -> void {
  try {

    std::cout << "\nBEGIN copy_command::execute\n\n";

    create_dirctories(snapshot_.directories(), destination_root_);
    rng::for_each(snapshot_.files(), [&](auto const& entry) {
      auto const from_path = snapshot_.root() / entry.first; 
      auto const to_path = destination_root_ / entry.first;
      std::cout << "copy from " << from_path << "to " << to_path << '\n';
      fs::copy(from_path, to_path, fs::copy_options::skip_existing);
    });

    std::cout << "\nEND copy_command::execute\n\n";

  } catch (fs::filesystem_error const& err) {
    throw_exception<errorcode::filesystem>(
      errorcode::filesystem::copy_command_failed,
      "execute", err.path1().string(),
      err.path2().string(), 
      err.what()
    );
  }
}

auto copy_command::undo() const -> void {
  try {

    std::cout << "\nBEGIN copy_command::undo\n\n";

    rng::for_each(snapshot_.files(), [&](auto const& entry) {
      auto const entry_path = destination_root_ / entry.first;
      fs::remove(entry_path);
      std::cout << "remove " << entry_path << '\n';
    });
    remove_directories(snapshot_.directories(), destination_root_);

    std::cout << "\nEND copy_command::undo\n\n";

  } catch (fs::filesystem_error const& err) {
    throw_exception<errorcode::filesystem>(
      errorcode::filesystem::copy_command_failed,
      "undo", err.path1().string(), 
      err.path2().string(), 
      err.what()
    );
  }
}

auto rename_command::execute() const -> void {
  try {

    std::cout << "\nBEGIN rename_command::execute\n\n";

    if (!fs::exists(destination_root_) && !snapshot_.files().empty()) { 
      fs::create_directory(destination_root_); 
    }

    create_dirctories(snapshot_.directories(), destination_root_);

    rng::for_each(snapshot_.files(), [&](auto const& entry) {
      auto const from_path = snapshot_.root() / entry.first;
      auto const to_path = destination_root_ / entry.first;
      fs::rename(from_path, to_path);
      std::cout << "rename from " << from_path 
                << " to " << to_path << '\n';
    });

    std::cout << "\nEND rename_command::execute\n\n";

  } catch (fs::filesystem_error const& err) {
    throw_exception<errorcode::filesystem>(
      errorcode::filesystem::rename_command_failed,
      "execute", err.path1().string(),
      err.path2().string(),
      err.what()
    );
  }
}

auto rename_command::undo() const -> void {
  try {

    std::cout << "\nBEGIN rename_command::undo\n\n";

    rng::for_each(snapshot_.files(), [&](auto const& entry) {
      auto const from_path = destination_root_ / entry.first;
      auto const to_path = snapshot_.root() / entry.first;
      fs::rename(from_path, to_path);
      std::cout << "rename from " << from_path 
                << " to " << to_path << '\n';
    });

    remove_directories(snapshot_.directories(), destination_root_);

    fs::remove(destination_root_);
    std::cout << "remove directory: " << destination_root_ << '\n';

    std::cout << "\nEND rename_command::undo\n\n";

  } catch (fs::filesystem_error const& err) {
    throw_exception<errorcode::filesystem>(
      errorcode::filesystem::rename_command_failed,
      "undo", err.path1().string(),
      err.path2().string(),
      err.what()
    );
  }
}

auto remove_command::execute() const -> void {
  try {

    std::cout << "\nBEGIN remove_command::execute\n\n";

    rng::for_each(snapshot_.files(), [&](auto const& entry) {
      auto const entry_path = snapshot_.root() / entry.first;
      fs::remove(entry_path);
      std::cout << "remove_command: remove " << entry_path << '\n';
    });

    remove_directories(snapshot_.directories(), snapshot_.root());
  
    if (auto const& root_path = snapshot_.root(); fs::exists(root_path)) {
      fs::remove(root_path);
      std::cout << "remove_command: remove directory " << root_path << '\n';
    }

    std::cout << "\nEND remove_command::execute\n\n";

  } catch (fs::filesystem_error const& err) {
    throw_exception<errorcode::filesystem>(
      errorcode::filesystem::remove_command_failed,
      "execute", err.path1().string(),
      err.what()
    );
  }
}

auto remove_command::undo() const -> void {
  try {

  } catch (fs::filesystem_error const& err) {
    throw_exception<errorcode::filesystem>(
      errorcode::filesystem::remove_command_failed,
      "undo", err.path1().string(),
      err.what()
    );
  }
}

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