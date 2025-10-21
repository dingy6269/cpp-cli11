#pragma once
#include <string>
#include <optional>

namespace app::cli {
namespace cli_defaults {
    inline constexpr const char *APP_NAME = "cli11_test";
    inline constexpr const char *DEFAULT_FILENAME = "index.js";
}

struct RunConfig {
    std::string filename;
    bool watch;
};

class CliConfig {
public:
    static std::optional<RunConfig> parse(int argc, char *argv[]);
};
}