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

auto create_directories(path_snapshot::snapshot_directories const& directories, 
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
      if(auto const directory_path = source_root / entry.first; fs::exists(directory_path)) {
        logger.get(logger_id::sync)->info(
          utility::formatter<messagecode::command>::format(
            messagecode::command::remove_directory, 
            directory_path.string() 
        ));

        fs::remove(directory_path);
      }
  });
}

auto copy_command::execute() -> void {
  try {
    logger.get(logger_id::sync)->debug(
      utility::formatter<messagecode::command>::format(
        messagecode::command::enter_command, 
        "copy_command", "execute"
    ));

    create_directories(snapshot_.directories(), destination_root_);

    rng::for_each(snapshot_.files(), [&](auto const& entry) {
      if (auto const to_path = destination_root_ / entry.first; 
          !fs::exists(to_path)) {
        auto const from_path = snapshot_.root() / entry.first; 
  
        logger.get(logger_id::sync)->info(
          utility::formatter<messagecode::command>::format(
            messagecode::command::copy_file, 
            from_path.string(), 
            to_path.string()
        ));
  
        fs::copy(from_path, to_path);
      }
    });

    execute_status_ = command_status::success; 

    logger.get(logger_id::sync)->debug(
      utility::formatter<messagecode::command>::format(
        messagecode::command::leave_command, 
        "copy_command", "execute"
    ));
  } catch (fs::filesystem_error const& err) {
    execute_status_ = command_status::failure; 

    throw_exception<errorcode::command>(
      errorcode::command::copy_command_failed,
      "execute", err.path1().string(),
      err.path2().string(), 
      err.what()
    );
  }
}

auto copy_command::undo() -> void {
  try {
    logger.get(logger_id::sync)->debug(
      utility::formatter<messagecode::command>::format(
        messagecode::command::enter_command, 
        "copy_command", "undo"
    ));

    if (execute_status_ == command_status::failure) {
      logger.get(logger_id::sync)->warn(
        utility::formatter<messagecode::command>::format(
          messagecode::command::undo_before_execute,
          "copy_command"
      ));

      logger.get(logger_id::sync)->debug(
        utility::formatter<messagecode::command>::format(
          messagecode::command::leave_command, 
          "copy_command", "undo"
      ));

      return;
    }

    auto& files = snapshot_.files();
    rng::for_each(files, [&](auto const& entry) {
      if (auto const entry_path = destination_root_ / entry.first; 
          fs::exists(entry_path)) {
        logger.get(logger_id::sync)->info(
          utility::formatter<messagecode::command>::format(
            messagecode::command::remove_file, 
            entry_path.string() 
        ));
  
        fs::remove(entry_path);
        files.extract(entry.first);
      }
    });

    auto& directories = snapshot_.directories(); 
    rng::for_each(rng::crbegin(directories), rng::crend(directories), 
      [&] (auto const& entry) { 
        if(auto const directory_path = destination_root_ / entry.first; 
           fs::exists(directory_path)) {
          logger.get(logger_id::sync)->info(
            utility::formatter<messagecode::command>::format(
              messagecode::command::remove_directory, 
              directory_path.string() 
          ));
  
          fs::remove(directory_path);
          directories.extract(entry.first);
        }
    });

    undo_status_ = command_status::success;

    logger.get(logger_id::sync)->debug(
      utility::formatter<messagecode::command>::format(
        messagecode::command::leave_command, 
        "copy_command", "undo"
    ));
  } catch (fs::filesystem_error const& err) {
    undo_status_ = command_status::failure;

    logger.get(logger_id::sync)->error(
      utility::formatter<errorcode::command>::format(
        errorcode::command::copy_command_failed,
        "undo", err.path1().string(), 
        err.path2().string(), 
        err.what()
    ));
  } catch (std::exception const& err) {
    undo_status_ = command_status::failure;

    logger.get(logger_id::sync)->error(
      utility::formatter<errorcode::system>::format(
        errorcode::system::unhandled_std_exception,
        err.what()
    ));
  }
}

auto rename_command::execute() -> void {
  try {
    logger.get(logger_id::sync)->debug(
      utility::formatter<messagecode::command>::format(
        messagecode::command::enter_command, 
        "rename_command", "execute"
    ));

    auto const& files = snapshot_.files();
    auto const& directories = snapshot_.directories();

    if (files.empty() && directories.empty()) { 
      logger.get(logger_id::sync)->debug(
        utility::formatter<messagecode::command>::format(
          messagecode::command::leave_command, 
          "rename_command", "execute"
      ));
      return; 
    }

    if (!fs::exists(destination_root_)) { 
      logger.get(logger_id::sync)->info(
        utility::formatter<messagecode::command>::format(
          messagecode::command::create_directory,
          destination_root_.string()
      ));
      fs::create_directory(destination_root_); 
    }

    create_directories(directories, destination_root_);

    rng::for_each(files, [&](auto const& entry) {
      if (auto const from_path = snapshot_.root() / entry.first; 
          fs::exists(from_path)) {
        auto const to_path = destination_root_ / entry.first;

        logger.get(logger_id::sync)->info(
          utility::formatter<messagecode::command>::format(
            messagecode::command::rename_file, 
            from_path.string(), 
            to_path.string()
        ));

        fs::rename(from_path, to_path);
      }
    });

    execute_status_ = command_status::success; 

    logger.get(logger_id::sync)->debug(
      utility::formatter<messagecode::command>::format(
        messagecode::command::leave_command, 
        "rename_command", "execute"
    ));
  } catch (fs::filesystem_error const& err) {
    execute_status_ = command_status::failure; 
    
    throw_exception<errorcode::command>(
      errorcode::command::rename_command_failed,
      "execute", err.path1().string(),
      err.path2().string(),
      err.what()
    );
  }
}

auto rename_command::undo() -> void {
  try {
    logger.get(logger_id::sync)->debug(
      utility::formatter<messagecode::command>::format(
        messagecode::command::enter_command, 
        "rename_command", "undo"
    ));

    if (execute_status_ == command_status::failure) {
      logger.get(logger_id::sync)->warn(
        utility::formatter<messagecode::command>::format(
          messagecode::command::undo_before_execute,
          "rename_command"
      ));

      logger.get(logger_id::sync)->debug(
        utility::formatter<messagecode::command>::format(
          messagecode::command::leave_command, 
          "rename_command", "undo"
      ));

      return;
    }

    auto& files = snapshot_.files();
    rng::for_each(files, [&](auto const& entry) {
      if (auto const from_path = destination_root_ / entry.first; 
          fs::exists(from_path)) {
        auto const to_path = snapshot_.root() / entry.first;

        logger.get(logger_id::sync)->info(
          utility::formatter<messagecode::command>::format(
            messagecode::command::rename_file, 
            from_path.string(), 
            to_path.string() 
        ));
  
        fs::rename(from_path, to_path);
        files.extract(entry.first);
      }
    });

    auto& directories = snapshot_.directories(); 
    rng::for_each(rng::crbegin(directories), rng::crend(directories), 
      [&] (auto const& entry) { 
        if(auto const directory_path = destination_root_ / entry.first; 
           fs::exists(directory_path)) {
          logger.get(logger_id::sync)->info(
            utility::formatter<messagecode::command>::format(
              messagecode::command::remove_directory, 
              directory_path.string() 
          ));
  
          fs::remove(directory_path);
          directories.extract(entry.first);
        }
    });

    if (fs::exists(destination_root_)) { 
      logger.get(logger_id::sync)->info(
        utility::formatter<messagecode::command>::format(
          messagecode::command::remove_directory, 
          destination_root_.string()
      ));
      fs::remove(destination_root_);
    }
    
    undo_status_ = command_status::success;

    logger.get(logger_id::sync)->debug(
      utility::formatter<messagecode::command>::format(
        messagecode::command::leave_command, 
        "rename_command", "undo"
    ));
  } catch (fs::filesystem_error const& err) {
    undo_status_ = command_status::failure;

    logger.get(logger_id::sync)->error(
      utility::formatter<errorcode::command>::format(
        errorcode::command::rename_command_failed,
        "undo", err.path1().string(), 
        err.path2().string(), 
        err.what()
    ));
  } catch (std::exception const& err) {
    undo_status_ = command_status::failure;

    logger.get(logger_id::sync)->error(
      utility::formatter<errorcode::system>::format(
        errorcode::system::unhandled_std_exception,
        err.what()
    ));
  }
}

auto remove_command::execute() -> void {
  try {
    logger.get(logger_id::sync)->debug(
      utility::formatter<messagecode::command>::format(
        messagecode::command::enter_command, 
        "remove_command", "execute"
    ));

    if (!fs::exists(snapshot_.root())) { 
      logger.get(logger_id::sync)->debug(
        utility::formatter<messagecode::command>::format(
          messagecode::command::leave_command, 
          "remove_command", "execute"
      ));
      return; 
    }
   
    auto const trash_path = snapshot_.root() / fs::path{".trash"};

    if (!fs::exists(trash_path)) { 
      logger.get(logger_id::sync)->info(
        utility::formatter<messagecode::command>::format(
          messagecode::command::create_directory,
          trash_path.string()
      ));
      fs::create_directory(trash_path); 
    }

    create_directories(snapshot_.directories(), trash_path);

    rng::for_each(snapshot_.files(), [&](auto const& entry) {
      auto const to_path = trash_path / entry.first; 
      auto const from_path = snapshot_.root() / entry.first; 
  
      logger.get(logger_id::sync)->info(
        utility::formatter<messagecode::command>::format(
          messagecode::command::copy_file, 
          from_path.string(), 
          to_path.string()
      ));
  
      fs::copy(from_path, to_path, fs::copy_options::overwrite_existing);
    });

    rng::for_each(snapshot_.files(), [&](auto const& entry) {
      if (auto const entry_path = snapshot_.root() / entry.first; 
          fs::exists(entry_path)) {
        logger.get(logger_id::sync)->info(
          utility::formatter<messagecode::command>::format(
            messagecode::command::remove_file, 
            entry_path.string()
        ));
  
        fs::remove(entry_path);
      }
    });

    remove_directories(snapshot_.directories(), snapshot_.root());

    execute_status_ = command_status::success; 

    try {
      logger.get(logger_id::sync)->info(
        utility::formatter<messagecode::command>::format(
          messagecode::command::remove_directory,
          snapshot_.root().string()
      ));
      fs::remove_all(snapshot_.root());
    } catch (fs::filesystem_error const& err) {
      execute_status_ = command_status::partial_success;

      logger.get(logger_id::sync)->warn(
        errorcode::command::remove_command_cleanup_failed,
        "execute", trash_path.string(), 
        err.what()
      );
    }

    logger.get(logger_id::sync)->debug(
      utility::formatter<messagecode::command>::format(
        messagecode::command::leave_command, 
        "remove_command", "execute"
    ));
  } catch (fs::filesystem_error const& err) {
    execute_status_ = command_status::failure; 

    throw_exception<errorcode::command>(
      errorcode::command::remove_command_failed,
      "execute", err.path1().string(),
      err.what()
    );
  }
}

auto remove_command::undo() -> void {
  try {
    logger.get(logger_id::sync)->debug(
      utility::formatter<messagecode::command>::format(
        messagecode::command::enter_command, 
        "remove_command", "undo"
    ));

    if (execute_status_ != command_status::success) {
      logger.get(logger_id::sync)->warn(
        utility::formatter<messagecode::command>::format(
          messagecode::command::undo_before_execute,
          "remove_command"
      ));

      logger.get(logger_id::sync)->debug(
        utility::formatter<messagecode::command>::format(
          messagecode::command::leave_command, 
          "remove_command", "undo"
      ));

      return;
    }

    auto const& trash_path = snapshot_.root() / fs::path{".trash"};

    if (!fs::exists(trash_path)) {
      logger.get(logger_id::sync)->debug(
        utility::formatter<messagecode::command>::format(
          messagecode::command::leave_command, 
          "remove_command", "undo"
      ));
      return;
    }

    auto& directories = snapshot_.directories(); 
    rng::for_each(directories, [&] (auto const& entry) { 
        if(auto const directory_path = snapshot_.root() / entry.first; 
           !fs::exists(directory_path)) {
          logger.get(logger_id::sync)->info(
            utility::formatter<messagecode::command>::format(
              messagecode::command::create_directory, 
              directory_path.string() 
          ));
  
          fs::create_directory(directory_path);
          directories.extract(entry.first);
        }
    });

    auto& files = snapshot_.files();
    rng::for_each(files, [&](auto const& entry) {
      if (auto const to_path = snapshot_.root() / entry.first; 
          !fs::exists(to_path)) {
        auto const from_path = trash_path / entry.first; 
  
        logger.get(logger_id::sync)->info(
          utility::formatter<messagecode::command>::format(
            messagecode::command::copy_file, 
            from_path.string(), 
            to_path.string()
        ));
  
        fs::copy(from_path, to_path);
        files.extract(entry.first);
      }
    });

    try {
      logger.get(logger_id::sync)->info(
        utility::formatter<messagecode::command>::format(
          messagecode::command::remove_directory,
          trash_path.string()
      ));

      fs::remove_all(trash_path);
    } catch (fs::filesystem_error const& err) {
      undo_status_ = command_status::partial_success;

      logger.get(logger_id::sync)->warn(
        errorcode::command::remove_command_cleanup_failed,
        "execute", trash_path.string(), 
        err.what()
      );
    }

    undo_status_ = command_status::success;

    logger.get(logger_id::sync)->debug(
      utility::formatter<messagecode::command>::format(
        messagecode::command::leave_command, 
        "remove_command", "undo"
    ));
  } catch (fs::filesystem_error const& err) {
    undo_status_ = command_status::failure;

    logger.get(logger_id::sync)->error(
      utility::formatter<errorcode::command>::format(
        errorcode::command::remove_command_failed,
        "undo", err.path1().string(), 
        err.what()
    ));
  } catch (std::exception const& err) {
    undo_status_ = command_status::failure;

    logger.get(logger_id::sync)->error(
      utility::formatter<errorcode::system>::format(
        errorcode::system::unhandled_std_exception,
        err.what()
    ));
  }
}

auto clone_transaction::start() -> void {
  if (commands_.empty()) { return; }
  rng::for_each(commands_, [&](auto& command) {
    try {
      std::visit([](auto& cmd) { cmd.execute(); }, command);
      processed_commands_.push(command);
    } catch (std::exception const& err) {
      std::visit([](auto& cmd) { cmd.undo(); }, command);
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
    std::visit([](auto& cmd) { cmd.undo(); }, command);
    processed_commands_.pop();
  }
}

} // namespace dropclone  