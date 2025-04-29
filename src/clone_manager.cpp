#include <dropclone/clone_manager.hpp>
#include <dropclone/clone_config.hpp>
#include <dropclone/path_snapshot.hpp>
#include <dropclone/exception.hpp>
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

auto clone_manager::copy(path_snapshot const& snapshot) -> void {

}
auto clone_manager::remove(path_snapshot const& snapshot) -> void {}

auto clone_manager::sync() -> void {
  try {
    auto current_source_snapshot = path_snapshot{source_snapshot_.root()};
    current_source_snapshot.make([&](fs::path const& path) { return entry_.filter(path); });

    if(source_snapshot_.hash() == current_source_snapshot.hash()) { return; }

    auto diff_snapshot_update = current_source_snapshot - source_snapshot_;
    copy(diff_snapshot_update);
  
    // if (entry_.mode == clone_mode::move) { remove(diff_snapshot_update); } 
  
    // auto diff_snapshot_remove = source_snapshot_ - current_source_snapshot; 
    // remove(diff_snapshot_remove);
  
    source_snapshot_ = std::move(current_source_snapshot);

  } catch (dc::exception const& e) {
    std::cerr << e.what() << '\n';
  }
}

} // dropclone

