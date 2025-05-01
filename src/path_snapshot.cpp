#include <dropclone/path_snapshot.hpp>
#include <dropclone/exception.hpp>
#include <dropclone/errorcode.hpp>
#include <ranges>
#include <algorithm>
#include <filesystem>
#include <numeric>
#include <execution>
#include <utility>
#include <functional>
#include <system_error>

#include <iostream> // remove it

namespace dropclone {

  namespace rng = std::ranges;
  namespace vws = std::views;
  namespace fs = std::filesystem;

  path_snapshot::path_snapshot(fs::path root) : root_{std::move(root)} {}

  auto path_snapshot::operator-(path_snapshot const& other) -> path_snapshot { 
    path_snapshot result{root_};

    auto const emplace = [&](fs::path const& path, path_info const& info) {
      if (fs::is_directory(root_ / path)) {
        result.directories_.emplace(path, info);
      } else {
        result.files_.emplace(path, info);
      }
    };

    rng::for_each(entries_, [&](auto& entry) {
      if (auto found = other.entries_.find(entry.first); found == rng::end(other.entries_)) {
        if (auto found_uncertain_paths = other.uncertain_processing_paths_.find(entry.first); 
            found_uncertain_paths != rng::end(other.uncertain_processing_paths_)) {
          result.uncertain_processing_paths_.insert(entry.first);
        } else {
          entry.second.path_status = path_info::status::added;
          emplace(entry.first, entry.second);
        }
      } else {
        entry.second.path_status = path_info::status::updated;
        if (entry.second.last_write_time != found->second.last_write_time) {
          entry.second = found->second;
          entry.second.path_status = path_info::status::updated;
          emplace(entry.first, entry.second);
        } else if (entry.second.last_write_time == found->second.last_write_time && 
                   entry.second.file_size != found->second.file_size) {
          entry.second.conflict = path_conflict_t::size_mismatch;
          result.conflicts_.emplace(entry.first, entry.second);
        } else if (entry.second.file_perms != found->second.file_perms) {
          entry.second.conflict = path_conflict_t::permission_mismatch;
          result.conflicts_.emplace(entry.first, entry.second);
        }
      } 
    });
    return result; 
  }

  auto path_snapshot::make(path_filter filter) -> void { 
    try {
      std::error_code error_code{};
      auto recursive_directory_view = 
        fs::recursive_directory_iterator{root_, fs::directory_options::none, error_code} 
        | vws::filter([&](auto const& entry) { return filter(entry.path()); });

      for (auto const& dir_entry : recursive_directory_view) {
        auto const relative_entry_path = fs::relative(dir_entry.path(), root_);
        if (error_code == std::errc::permission_denied) {
          uncertain_processing_paths_.insert(relative_entry_path);
          error_code.clear();
        } else {
          entries_.try_emplace(relative_entry_path,
            dir_entry.last_write_time(), 
            dir_entry.is_directory() ? 0 : dir_entry.file_size(), 
            dir_entry.status().permissions()
          ); 
        }
      }
    } catch (fs::filesystem_error const& e) {
      throw_exception<errorcode::filesystem>(
        errorcode::filesystem::failed_to_traverse_directory,
        root_.string(),
        "path_snapshot::make()",
        e.what()
      );
    } catch (std::exception const& e) {
      throw_exception<errorcode::system>(
        errorcode::system::unhandled_std_exception,
        e.what()
      );
    }
    hash_ = compute_hash(); 
  }
  
  auto path_snapshot::compute_hash() const -> size_t {
    return std::reduce(std::execution::par, rng::begin(entries_), rng::end(entries_), size_t{0}, 
      [](size_t seed, auto const& file) {
        return std::bit_xor{}(seed, std::hash<path_info>{}(file.second));
    }); 
  }

  auto path_snapshot::add_files(snapshot_entries const& entries, entry_filter filter) -> void {
    rng::for_each(entries, [&](auto const& entry) {
      if (filter(entry)) { files_.emplace(entry.first, entry.second); }
    });
  }

  auto path_snapshot::add_directories(snapshot_directories const& directories, entry_filter filter) -> void {
    rng::for_each(directories, [&](auto const& directory) {
      if (filter(directory)) { directories_.emplace(directory.first, directory.second); }
    });
  }

  auto path_snapshot::rebase(fs::path const& new_root) -> void {
    root_ = new_root;
  }

} // namespace dropclone