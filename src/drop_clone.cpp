#include <drop_clone.hpp>
#include <clone_config.hpp>
#include <exception.hpp>
#include <utility>
#include <fstream>
#include <filesystem>
#include <iostream> // will be removed later

namespace dropclone {

// maybe is config_path not neccessary as member
drop_clone::drop_clone(fs::path config_path, config_parser parser) 
    : config_path_{std::move(config_path)}, parser_{std::move(parser)} {

  try {
    clone_config_ = parser_(config_path_);

    for (auto& entry : clone_config_.entries) {
      entry.sanitize();
    }

    clone_config_.validate();

  } catch (dropclone::exception const& e) {
    std::cerr << e.what();
    std::exit(EXIT_FAILURE);
  } catch (std::exception const& e) {
    std::cerr << "Unhandled std::exception: " << e.what();
    std::exit(EXIT_FAILURE);
  } catch (...) {
    std::cerr << "Unknown fatal error occurred.";
    std::exit(EXIT_FAILURE);
  }
}
  
} // namespace dropclone