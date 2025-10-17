#pragma once

#include <string>
#include <optional>
#include <CLI/CLI.hpp>

namespace cli_defaults {
inline constexpr const char *APP_NAME = "cherry";
inline constexpr const char *DEFAULT_FILENAME = "index.js";
} // namespace cli_defaults

struct RunConfig {
  std::string filename;
  bool watch;
};

class CliConfig {
public:
  static std::optional<RunConfig> parse(int argc, char *argv[]) {
    CLI::App app{cli_defaults::APP_NAME};

    auto run = app.add_subcommand("run", "run file");
    std::string filename;
    bool watch;

    run->add_option("-n, --name", filename, "file name")
        ->default_val(cli_defaults::DEFAULT_FILENAME);
    // here was an error (two dashes)
    run->add_flag("-w", watch, "watch dir")->default_val(false);

    try {
      app.parse(argc, argv);
    } catch (const CLI::CallForAllHelp &e) {
      std::cout << app.help() << std::endl;
      return std::nullopt;
    } catch (const CLI::ParseError &e) {
      app.exit(e);
      return std::nullopt;
    };

    if (app.got_subcommand(run)) {
      return RunConfig{
        filename,
        watch
      };
    }

    return std::nullopt;
  }
};