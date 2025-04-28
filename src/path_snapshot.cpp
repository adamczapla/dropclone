#include <dropclone/path_snapshot.hpp>
#include <dropclone/exception.hpp>
#include <dropclone/errorcode.hpp>
#include <ranges>
#include <filesystem>
#include <numeric>
#include <execution>
#include <utility>
#include <functional>

#include <iostream> // remove it

namespace dropclone {

  namespace rng = std::ranges;
  namespace vws = std::views;
  namespace fs = std::filesystem;

  path_snapshot::path_snapshot(fs::path root) : root_{std::move(root)} {}

  auto path_snapshot::operator-(path_snapshot const& other) -> path_snapshot { 
    path_snapshot result{root_};
    // rng::for_each(entries_, [&](auto const& entry) {
    //   if (auto found = other.entries_.find(entry.first); found == rng::end(other.entries_)) {
    //     result.entries_.emplace(entry.first, entry.second);
    //   } else {
    //     if (entry.second.last_write_time < found->second.last_write_time) {
    //       result.entries_.emplace(found->first, found->second);
    //     } else if (entry.second.last_write_time == found->second.last_write_time && 
    //                entry.second.file_size != found->second.file_size) {
    //       result.conflicts_.emplace(found->first, found->second);
    //     }
    //   } 
    // });
    return result; 
  }
  
  auto path_snapshot::make(path_filter filter) -> void { 
    try {
      auto recursive_directory_view = 
        fs::recursive_directory_iterator{root_, fs::directory_options::skip_permission_denied} 
        | vws::filter([&](auto const& entry) { return filter(entry.path()); });

      for (auto const& dir_entry : recursive_directory_view) {
        entries_.try_emplace(dir_entry.path(), 
                             dir_entry.last_write_time(), 
                             dir_entry.is_directory() ? 0 : dir_entry.file_size(), 
                             dir_entry.status().permissions()
        ); 
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

} // namespace dropclone