#include <dropclone.hpp>
#include <numeric>
#include <execution>
#include <functional>

namespace dropclone {

namespace rng = std::ranges;

auto hash_snapshot(file_snapshot const& snapshot) noexcept -> size_t {
  return std::reduce(std::execution::par, rng::begin(snapshot), rng::end(snapshot), size_t{0}, 
    [](size_t seed, auto const& file) {
      return std::bit_xor{}(seed, std::hash<file_info>{}(file.second));
    }); 
}

} // dropclone

