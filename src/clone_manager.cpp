#include <dropclone/clone_manager.hpp>
#include <dropclone/clone_config.hpp>
#include <dropclone/path_snapshot.hpp>
#include <dropclone/exception.hpp>
#include <dropclone/clone_transaction.hpp>
#include <filesystem>

#include <iostream> // do not forget to remove it

namespace dropclone {

namespace fs = std::filesystem;
namespace dc = dropclone;

clone_manager::clone_manager(config_entry entry) 
  : source_snapshot_{entry.source_directory}, 
    destination_snapshot_{entry.destination_directory}, 
    entry_{std::move(entry)}
{}

auto clone_manager::copy(path_snapshot const& source_snapshot, fs::path const& destination_root) -> void {
  auto const filter_added_path = [](auto const& entry) -> bool { 
      return entry.second.path_status == path_info::status::added; 
  };
  path_snapshot added_paths{source_snapshot.root()};
  added_paths.add_files(source_snapshot.files(), filter_added_path);
  added_paths.add_directories(source_snapshot.directories(), filter_added_path);
  copy_command copy_added_paths{added_paths, destination_root};

  auto const filter_updated_path = [](auto const& entry) -> bool { 
      return entry.second.path_status == path_info::status::updated;
  };
  path_snapshot updated_paths{source_snapshot.root()};
  updated_paths.add_files(source_snapshot.files(), filter_updated_path);
  updated_paths.add_directories(source_snapshot.directories(), filter_updated_path);
  
  path_snapshot renamed_paths = updated_paths;
  renamed_paths.rebase(destination_root);
  auto const backup_path = destination_root / fs::path{".backup"};
  rename_command rename_updated_paths{renamed_paths, backup_path}; 

  updated_paths.clear_directories();
  copy_command copy_updated_paths{updated_paths, destination_root};
  renamed_paths.rebase(backup_path);
  remove_command remove_renamed_paths{renamed_paths};

  clone_transaction copy_transaction{};
  copy_transaction.add(copy_added_paths);
  copy_transaction.add(rename_updated_paths);
  copy_transaction.add(copy_updated_paths);
  copy_transaction.add(remove_renamed_paths);
  try {
    copy_transaction.start();
  } catch (dropclone::exception const& err) {
    //
  }
}
auto clone_manager::remove(path_snapshot const& snapshot) -> void {}

auto clone_manager::sync() -> void {
  try {
    auto current_source_snapshot = path_snapshot{source_snapshot_.root()};
    current_source_snapshot.make([&](fs::path const& path) { return entry_.filter(path); });

    if(source_snapshot_.hash() == current_source_snapshot.hash()) { return; }

    auto diff_snapshot_update = current_source_snapshot - source_snapshot_;
    copy(diff_snapshot_update, entry_.destination_directory);
  
    // if (entry_.mode == clone_mode::move) { remove(diff_snapshot_update); } 
  
    // auto diff_snapshot_remove = source_snapshot_ - current_source_snapshot; 
    // remove(diff_snapshot_remove, entry_.destination_directory);
  
    source_snapshot_ = std::move(current_source_snapshot);

  } catch (dc::exception const& e) {
    std::cerr << e.what() << '\n';
  }
}

} // dropclone

