#include <dropclone/clone_transaction.hpp>
#include <dropclone/path_snapshot.hpp>
#include <dropclone/exception.hpp>
#include <filesystem>
#include <variant>
#include <stdexcept>
#include <algorithm>

namespace dropclone {

namespace rng = std::ranges;
namespace fs = std::filesystem;

auto copy_command::execute() const -> void {
  try {
    // copy code
  } catch (fs::filesystem_error const& err) {
    throw_exception<errorcode::filesystem>(
      errorcode::filesystem::copy_command_failed,
      err.path1().string(),
      err.path2().string(),
      err.what()
    );
  }
}
auto copy_command::undo() const -> void {}

auto rename_command::execute() const -> void {}
auto rename_command::undo() const -> void {}

auto remove_command::execute() const -> void {}
auto remove_command::undo() const -> void {}

auto clone_transaction::start() -> void {
  if (commands_.empty()) { return; }
  rng::for_each(commands_, [&](auto const& command) {
    try {
      std::visit([](auto const& cmd) { cmd.execute(); }, command);
      processed_commands_.push(command);
    } catch (std::exception const& err) {
      std::visit([](auto const& cmd) { cmd.undo(); }, command);
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

}

} // namespace dropclone  