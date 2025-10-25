#pragma once
#include <optional>
#include <string>

namespace app::cli {
namespace cli_defaults {
inline constexpr const char *APP_NAME = "cli11_test";
inline constexpr const char *DEFAULT_FILENAME = "index.js";
} // namespace cli_defaults

struct RunConfig {
  std::string filename;
  bool watch;
};

class CliConfig {
public:
  static std::optional<RunConfig> parse(int argc, char *argv[]);
};
} // namespace app::cli