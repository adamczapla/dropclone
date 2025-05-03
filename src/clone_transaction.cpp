#include <dropclone/clone_transaction.hpp>
#include <dropclone/path_snapshot.hpp>
#include <dropclone/logger_manager.hpp>
#include <dropclone/utility.hpp>
#include <dropclone/errorcode.hpp>
#include <dropclone/messagecode.hpp>
#include <dropclone/exception.hpp>
#include <filesystem>
#include <variant>
#include <stdexcept>
#include <algorithm>

namespace dropclone {

namespace rng = std::ranges;
namespace fs = std::filesystem;

auto create_dirctories(path_snapshot::snapshot_directories const& directories, 
                       fs::path const& destination_root) -> void {
  rng::for_each(directories, [&](auto const& entry) {
    if(auto const directory = destination_root / entry.first; !fs::exists(directory)) {
      logger.get(logger_id::sync)->info(
        utility::formatter<messagecode::command>::format(
          messagecode::command::create_directory, 
          directory.string()
      ));

      fs::create_directories(directory);
    }
  });
}

auto remove_directories(path_snapshot::snapshot_directories const& directories,
                        fs::path const& source_root) -> void {
  rng::for_each(rng::crbegin(directories), rng::crend(directories),
    [&] (auto const& entry) { 
      auto const directory_path = source_root / entry.first;

      logger.get(logger_id::sync)->info(
        utility::formatter<messagecode::command>::format(
          messagecode::command::remove_directory, 
          directory_path.string() 
      ));

      fs::remove(directory_path);
  });
}

auto copy_command::execute() const -> void {
  try {
    logger.get(logger_id::sync)->debug(
      utility::formatter<messagecode::command>::format(
        messagecode::command::enter_command, 
        "copy_command", "execute"
    ));

    create_dirctories(snapshot_.directories(), destination_root_);
    rng::for_each(snapshot_.files(), [&](auto const& entry) {
      auto const from_path = snapshot_.root() / entry.first; 
      auto const to_path = destination_root_ / entry.first;

      logger.get(logger_id::sync)->info(
        utility::formatter<messagecode::command>::format(
          messagecode::command::copy_file, 
          from_path.string(), 
          to_path.string()
      ));

      fs::copy(from_path, to_path, fs::copy_options::skip_existing);
    });

    logger.get(logger_id::sync)->debug(
      utility::formatter<messagecode::command>::format(
        messagecode::command::leave_command, 
        "copy_command", "execute"
    ));
  } catch (fs::filesystem_error const& err) {
    throw_exception<errorcode::command>(
      errorcode::command::copy_command_failed,
      "execute", err.path1().string(),
      err.path2().string(), 
      err.what()
    );
  }
}

auto copy_command::undo() const -> void {
  try {
    logger.get(logger_id::sync)->debug(
      utility::formatter<messagecode::command>::format(
        messagecode::command::enter_command, 
        "copy_command", "undo"
    ));

    rng::for_each(snapshot_.files(), [&](auto const& entry) {
      auto const entry_path = destination_root_ / entry.first;

      logger.get(logger_id::sync)->info(
        utility::formatter<messagecode::command>::format(
          messagecode::command::remove_file, 
          entry_path.string() 
      ));

      fs::remove(entry_path);
    });

    remove_directories(snapshot_.directories(), destination_root_);

    logger.get(logger_id::sync)->debug(
      utility::formatter<messagecode::command>::format(
        messagecode::command::leave_command, 
        "copy_command", "undo"
    ));
  } catch (fs::filesystem_error const& err) {
    logger.get(logger_id::sync)->error(
      utility::formatter<errorcode::command>::format(
        errorcode::command::copy_command_failed,
        "undo", err.path1().string(), 
        err.path2().string(), 
        err.what()
    ));
  }
}

auto rename_command::execute() const -> void {
  try {
    logger.get(logger_id::sync)->debug(
      utility::formatter<messagecode::command>::format(
        messagecode::command::enter_command, 
        "rename_command", "execute"
    ));

    if (!fs::exists(destination_root_) && !snapshot_.files().empty()) { 
      fs::create_directory(destination_root_); 
    }

    create_dirctories(snapshot_.directories(), destination_root_);

    rng::for_each(snapshot_.files(), [&](auto const& entry) {
      auto const from_path = snapshot_.root() / entry.first;
      auto const to_path = destination_root_ / entry.first;

      logger.get(logger_id::sync)->info(
        utility::formatter<messagecode::command>::format(
          messagecode::command::rename_file, 
          from_path.string(), 
          to_path.string()
      ));

      fs::rename(from_path, to_path);
    });

    logger.get(logger_id::sync)->debug(
      utility::formatter<messagecode::command>::format(
        messagecode::command::leave_command, 
        "rename_command", "execute"
    ));
  } catch (fs::filesystem_error const& err) {
    throw_exception<errorcode::command>(
      errorcode::command::rename_command_failed,
      "execute", err.path1().string(),
      err.path2().string(),
      err.what()
    );
  }
}

auto rename_command::undo() const -> void {
  try {

    logger.get(logger_id::sync)->debug(
      utility::formatter<messagecode::command>::format(
        messagecode::command::enter_command, 
        "rename_command", "undo"
    ));

    rng::for_each(snapshot_.files(), [&](auto const& entry) {
      auto const from_path = destination_root_ / entry.first;
      auto const to_path = snapshot_.root() / entry.first;

      logger.get(logger_id::sync)->info(
        utility::formatter<messagecode::command>::format(
          messagecode::command::rename_file, 
          from_path.string(), 
          to_path.string() 
      ));

      fs::rename(from_path, to_path);
    });

    remove_directories(snapshot_.directories(), destination_root_);

    logger.get(logger_id::sync)->info(
      utility::formatter<messagecode::command>::format(
        messagecode::command::remove_directory, 
        destination_root_.string()
    ));

    fs::remove(destination_root_);

    logger.get(logger_id::sync)->debug(
      utility::formatter<messagecode::command>::format(
        messagecode::command::leave_command, 
        "rename_command", "undo"
    ));
  } catch (fs::filesystem_error const& err) {
    logger.get(logger_id::sync)->error(
      utility::formatter<errorcode::command>::format(
        errorcode::command::rename_command_failed,
        "undo", err.path1().string(), 
        err.path2().string(), 
        err.what()
    ));
  }
}

auto remove_command::execute() const -> void {
  try {

    logger.get(logger_id::sync)->debug(
      utility::formatter<messagecode::command>::format(
        messagecode::command::enter_command, 
        "remove_command", "execute"
    ));

    rng::for_each(snapshot_.files(), [&](auto const& entry) {
      auto const entry_path = snapshot_.root() / entry.first;

      logger.get(logger_id::sync)->info(
        utility::formatter<messagecode::command>::format(
          messagecode::command::remove_file, 
          entry_path.string()
      ));

      fs::remove(entry_path);
    });

    remove_directories(snapshot_.directories(), snapshot_.root());
  
    if (auto const& root_path = snapshot_.root(); fs::exists(root_path)) {
      logger.get(logger_id::sync)->info(
        utility::formatter<messagecode::command>::format(
          messagecode::command::remove_directory, 
          root_path.string()
      ));

      fs::remove(root_path);
    }

    logger.get(logger_id::sync)->debug(
      utility::formatter<messagecode::command>::format(
        messagecode::command::leave_command, 
        "remove_command", "execute"
    ));
  } catch (fs::filesystem_error const& err) {
    throw_exception<errorcode::command>(
      errorcode::command::remove_command_failed,
      "execute", err.path1().string(),
      err.what()
    );
  }
}

auto remove_command::undo() const -> void {
  try {
    logger.get(logger_id::sync)->debug(
      utility::formatter<messagecode::command>::format(
        messagecode::command::enter_command, 
        "remove_command", "undo"
    ));

    logger.get(logger_id::sync)->debug(
      utility::formatter<messagecode::command>::format(
        messagecode::command::leave_command, 
        "remove_command", "undo"
    ));
  } catch (fs::filesystem_error const& err) {
    logger.get(logger_id::sync)->error(
      utility::formatter<errorcode::command>::format(
        errorcode::command::remove_command_failed,
        "undo", err.path1().string(), 
        err.what()
    ));
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