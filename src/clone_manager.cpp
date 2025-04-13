#include <clone_manager.hpp>
#include <filesystem>
#include <numeric>
#include <execution>
#include <functional>

namespace dropclone {

namespace rng = std::ranges;
namespace fs = std::filesystem;

clone_manager::clone_manager(fs::path root) : root_{root} { make_snapshot(); }

auto clone_manager::hash(path_snapshot const& snapshot) noexcept -> size_t {
  return std::reduce(std::execution::par, rng::begin(snapshot), rng::end(snapshot), size_t{0}, 
    [](size_t seed, auto const& file) {
      return std::bit_xor{}(seed, std::hash<path_info>{}(file.second));
    }); 
}

auto clone_manager::make_snapshot() -> void {

}

} // dropclone

