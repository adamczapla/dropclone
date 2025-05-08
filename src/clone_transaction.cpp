#include <dropclone/clone_transaction.hpp>
#include <dropclone/path_snapshot.hpp>
#include <dropclone/logger_manager.hpp>
#include <dropclone/utility.hpp>
#include <dropclone/errorcode.hpp>
#include <dropclone/messagecode.hpp>
#include <dropclone/exception.hpp>
#include <chrono>
#include <cstdint>
#include <thread>
#include <filesystem>
#include <variant>
#include <stdexcept>
#include <algorithm>
#include <functional>

namespace dropclone {

namespace rng = std::ranges;
namespace fs = std::filesystem;
namespace dc = dropclone;

auto log_enter_command(std::string_view command_name, 
                       std::string_view function_name) -> void {
  logger.get(logger_id::sync)->debug(
    utility::formatter<messagecode::command>::format(
      messagecode::command::enter_command, 
      command_name, function_name
  ));
}

auto log_leave_command(std::string_view command_name, 
                       std::string_view function_name) -> void {
  logger.get(logger_id::sync)->debug(
    utility::formatter<messagecode::command>::format(
      messagecode::command::leave_command, 
      command_name, function_name
  ));
}

auto create_directory(fs::path const& directory_path) -> void {
  if (!fs::exists(directory_path)) { 
    logger.get(logger_id::sync)->info(
      utility::formatter<messagecode::command>::format(
        messagecode::command::create_directory,
        directory_path.string()
    ));

    fs::create_directory(directory_path); 
  }
}

auto remove_directory(fs::path const& directory_path) -> void {
  if (fs::exists(directory_path)) { 
    logger.get(logger_id::sync)->info(
      utility::formatter<messagecode::command>::format(
        messagecode::command::remove_directory, 
        directory_path.string()
    ));

    fs::remove(directory_path);
  }
}

auto create_directories(path_snapshot::snapshot_directories& directories, 
                        fs::path const& destination_root, 
                        bool extract_on_success) -> void {
  rng::for_each(directories, [&](auto const& entry) {
    if(auto const directory_path = destination_root / entry.first; 
       !fs::exists(directory_path)) {

      logger.get(logger_id::sync)->info(
        utility::formatter<messagecode::command>::format(
          messagecode::command::create_directory, 
          directory_path.string()
      ));

      fs::create_directories(directory_path);
      if (extract_on_success) { 
        directories.extract(entry.first); 
      }
    }
  });
}

auto remove_directories(path_snapshot::snapshot_directories& directories,
                        fs::path const& source_root,
                        bool extract_on_success) -> void {
  rng::for_each(rng::crbegin(directories), rng::crend(directories),
    [&] (auto const& entry) { 
      if(auto const directory_path = source_root / entry.first; 
         fs::exists(directory_path)) {

        logger.get(logger_id::sync)->info(
          utility::formatter<messagecode::command>::format(
            messagecode::command::remove_directory, 
            directory_path.string() 
        ));

        fs::remove(directory_path);
        if (extract_on_success) {
          directories.extract(entry.first);
        }
      }
  });
}

auto copy_files(path_snapshot::snapshot_entries& files, 
                fs::path const& source_root, 
                fs::path const& destination_root,
                bool extract_on_success, 
                fs::copy_options options) -> void {
  rng::for_each(files, [&](auto const& entry) {
    if (auto const to_path = destination_root / entry.first; 
        !fs::exists(to_path)) {
      auto const from_path = source_root / entry.first; 

      logger.get(logger_id::sync)->info(
        utility::formatter<messagecode::command>::format(
          messagecode::command::copy_file, 
          from_path.string(), 
          to_path.string()
      ));

      fs::copy(from_path, to_path, options);
      if (extract_on_success) {
        files.extract(entry.first);
      }
    }
  });
}

auto rename_files(path_snapshot::snapshot_entries& files, 
                  fs::path const& source_root, 
                  fs::path const& destination_root,
                  bool extract_on_success) -> void {
  rng::for_each(files, [&](auto const& entry) {
    if (auto const from_path = source_root / entry.first; 
        fs::exists(from_path)) {

      auto const to_path = destination_root / entry.first;

      logger.get(logger_id::sync)->info(
        utility::formatter<messagecode::command>::format(
          messagecode::command::rename_file, 
          from_path.string(), 
          to_path.string() 
      ));

      fs::rename(from_path, to_path);
      if (extract_on_success) {
        files.extract(entry.first);
      }
    }
  });
}

auto remove_files(path_snapshot::snapshot_entries& files, 
                  fs::path const& source_root,
                  bool extract_on_success) -> void {
  rng::for_each(files, [&](auto const& entry) {
    if (auto const entry_path = source_root / entry.first; 
        fs::exists(entry_path)) {

      logger.get(logger_id::sync)->info(
        utility::formatter<messagecode::command>::format(
          messagecode::command::remove_file, 
          entry_path.string() 
      ));

      fs::remove(entry_path);
      if (extract_on_success) {
        files.extract(entry.first);
      }
    }
  });
}

auto command_base::execute(std::string_view command_name, 
                           std::string_view errorcode, 
                           std::function<void(void)> execute) -> void {
  try {
    log_enter_command(command_name, "execute");

    if (execute_status_ == command_status::failure ||
        undo_status_ == command_status::failure) {
      logger.get(logger_id::sync)->warn(
        utility::formatter<messagecode::command>::format(
          messagecode::command::execute_skipped,
          command_name
      ));
  
      log_leave_command(command_name, "execute");

      return;
    }
  
    execute();

    execute_status_ = command_status::success; 

    log_leave_command(command_name, "execute");
  } catch (fs::filesystem_error const& err) {
    execute_status_ = command_status::failure; 

    logger.get(logger_id::sync)->error(
      utility::formatter<errorcode::command>::format(
        errorcode, "execute", err.path1().string(),
        err.path2().string(), 
        err.what()
    ));

    throw_exception<errorcode::command>(
      errorcode, "execute", err.path1().string(),
      err.path2().string(), 
      err.what()
    );
  } catch (std::exception const& err) {
    execute_status_ = command_status::failure; 

    logger.get(logger_id::sync)->error(
      errorcode::system::unhandled_std_exception,
      err.what()
    );

    throw_exception<errorcode::system>(
      errorcode::system::unhandled_std_exception,
      err.what()
    );
  }
}


auto command_base::undo(std::string_view command_name, 
                        std::string_view errorcode,
                        std::function<void(void)> undo) -> void {
  try {
    log_enter_command(command_name, "undo");

    if (execute_status_ == command_status::uninitialized ||
        undo_status_ == command_status::success) {
      logger.get(logger_id::sync)->warn(
        utility::formatter<messagecode::command>::format(
          messagecode::command::undo_skipped,
          command_name
      ));

      log_leave_command(command_name, "undo");

      return;
    }

    undo();

    undo_status_ = command_status::success;

    log_leave_command(command_name, "undo");
  } catch (fs::filesystem_error const& err) {
    undo_status_ = command_status::failure;

    logger.get(logger_id::sync)->error(
      utility::formatter<errorcode::command>::format(
        errorcode, "undo", err.path1().string(), 
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

auto copy_command::execute() -> void {
  command_base::execute("copy_command", errorcode::command::copy_command_failed, 
    [&] {
      create_directories(snapshot_.directories(), destination_root_);
      copy_files(snapshot_.files(), snapshot_.root(), destination_root_);
    }
  );
}

auto copy_command::undo() -> void {
  command_base::undo("copy_command", errorcode::command::copy_command_failed,
    [&] {
      remove_files(snapshot_.files(), destination_root_, true);
      remove_directories(snapshot_.directories(), destination_root_, true);
    }
  );
}

auto rename_command::execute() -> void {
  command_base::execute("rename_command", errorcode::command::rename_command_failed, 
    [&] {
      auto& files = snapshot_.files();
      auto& directories = snapshot_.directories();
  
      if (files.empty() && directories.empty()) { 
        logger.get(logger_id::sync)->debug(
          utility::formatter<messagecode::command>::format(
            messagecode::command::leave_command, 
            "rename_command", "execute"
        ));
  
        return; 
      }
  
      dc::create_directory(destination_root_);
      create_directories(directories, destination_root_);
      rename_files(files, snapshot_.root(), destination_root_);
    }
  );
}

auto rename_command::undo() -> void {
  command_base::undo("rename_command", errorcode::command::rename_command_failed,
    [&] {
      rename_files(snapshot_.files(), destination_root_, snapshot_.root(), true);
      remove_directories(snapshot_.directories(), destination_root_, true);
      remove_directory(destination_root_);
    }
  );
}

auto remove_command::execute() -> void {
  command_base::execute("remove_command", errorcode::command::remove_command_failed, 
    [&] {
      if (!fs::exists(snapshot_.root())) { 
        logger.get(logger_id::sync)->debug(
          utility::formatter<messagecode::command>::format(
            messagecode::command::leave_command, 
            "remove_command", "execute"
        ));
  
        return; 
      }
    
      auto const& source_root = snapshot_.root();
      auto const trash_path = source_root / fs::path{".trash"};
  
      dc::create_directory(trash_path);
      create_directories(snapshot_.directories(), trash_path);
      copy_files(snapshot_.files(), source_root, trash_path, 
                false, fs::copy_options::overwrite_existing);
      remove_files(snapshot_.files(), snapshot_.root());
      remove_directories(snapshot_.directories(), snapshot_.root());
  
      execute_status_ = command_status::success; 
  
      logger.get(logger_id::sync)->info(
        utility::formatter<messagecode::command>::format(
          messagecode::command::remove_directory,
          snapshot_.root().string()
      ));
  
      fs::remove_all(snapshot_.root());
    }
  );
}

auto remove_command::undo() -> void {
  command_base::undo("remove_command", errorcode::command::remove_command_failed,
    [&] {
      auto const& trash_path = snapshot_.root() / fs::path{".trash"};

      if (!fs::exists(trash_path)) {
        logger.get(logger_id::sync)->debug(
          utility::formatter<messagecode::command>::format(
            messagecode::command::leave_command, 
            "remove_command", "undo"
        ));
  
        return;
      }
  
      create_directories(snapshot_.directories(), snapshot_.root(), true);
      copy_files(snapshot_.files(), trash_path, snapshot_.root(), true);
  
      logger.get(logger_id::sync)->info(
        utility::formatter<messagecode::command>::format(
          messagecode::command::remove_directory,
          trash_path.string()
      ));
  
      fs::remove_all(trash_path);
    }
  );
}

auto clone_transaction::try_undo(clone_command command, 
                                 std::uint8_t max_retries) -> void {
  std::visit([&](auto& cmd) { 
      cmd.undo(); 
      for (uint8_t retries{0}; cmd.undo_status_ == command_status::failure && 
                               retries != max_retries; ++retries) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        cmd.undo();
      }
  }, command);
}

auto clone_transaction::log_unrecovered_entries() -> void {
  rng::for_each(commands_, [&](auto& command) {
    std::visit([](auto& cmd) {
      if (cmd.undo_status_ == command_status::failure) {
        logger.get(logger_id::sync)->error(
          utility::formatter<errorcode::transaction>::format(
            errorcode::transaction::unrecovered_entries,
            cmd.snapshot_.root().string()
        ));
  
        rng::for_each(cmd.snapshot_.files(), [](auto const& file) {
          logger.get(logger_id::sync)->error(
            utility::formatter<errorcode::transaction>::format(
              errorcode::transaction::unrecovered_file,
              file.first.string()
          ));
        });
  
        rng::for_each(cmd.snapshot_.directories(), [](auto const& directory) {
          logger.get(logger_id::sync)->error(
            utility::formatter<errorcode::transaction>::format(
              errorcode::transaction::unrecovered_file,
              directory.first.string()
          ));
        });
      }
    }, command);
  });
}

auto clone_transaction::reset() -> void {
  decltype(processed_commands_){}.swap(processed_commands_);
  decltype(commands_){}.swap(commands_);
}

auto clone_transaction::reset_command_statuses() -> void {
  rng::for_each(commands_, [&](auto& command) {
    std::visit([](auto& cmd) {
      cmd.execute_status_ = command_status::uninitialized;
      cmd.undo_status_ = command_status::uninitialized;
    }, command);
  });
}

auto clone_transaction::start() -> void {
  if (commands_.empty()) { return; }

  try {
    rng::for_each(commands_, [&](auto& command) {
      try {
        std::visit([](auto& cmd) { cmd.execute(); }, command);
        processed_commands_.push(command);
      } catch (dc::exception const& err) {
        try_undo(command, 3);
        throw;
      }
    });
  } catch (dc::exception const& err) {
    rollback();

    auto has_failure = rng::any_of(commands_, [](auto const& command) {
      return std::visit([](auto const& cmd) { 
        return cmd.undo_status_ == command_status::failure;
      }, command);
    });

    if (has_failure) {
      logger.get(logger_id::sync)->error(
        utility::formatter<errorcode::transaction>::format(
          errorcode::transaction::rollback_failed,
          err.what()
      ));

      log_unrecovered_entries();
      reset();

      throw_exception<errorcode::transaction>(
        errorcode::transaction::rollback_failed,
        err.what()
      );
    } 
    
    reset_command_statuses();

    throw_exception<errorcode::transaction>(
      errorcode::transaction::start_failed,
      err.what()
    );
  }
}

auto clone_transaction::rollback() -> void {
  while (!processed_commands_.empty()) {
    auto command = processed_commands_.top();
    try_undo(command, 1);
    processed_commands_.pop();
  }
}

} // namespace dropclone  