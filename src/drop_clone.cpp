#include <drop_clone.hpp>
#include <utility>

namespace dropclone {

drop_clone::drop_clone(config_parser parser) noexcept : parser_{std::move(parser)} { }
  
} // namespace dropclone