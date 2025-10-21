#include "cli.h"

#include <CLI/CLI.hpp>
#include <iostream>


namespace app::cli {
std::optional<RunConfig> CliConfig::parse(int argc, char *argv[]) {
    CLI::App app{cli_defaults::APP_NAME};
    auto run = app.add_subcommand("run", "run file");
    
    std::string filename;
    bool watch;
    
    run->add_option("-n, --name", filename, "file name")
        ->default_val(cli_defaults::DEFAULT_FILENAME);
    
    run->add_flag("-w", watch, "watch dir")->default_val(false);
    
    try {
        app.parse(argc, argv);
    } catch (const CLI::CallForAllHelp &e) {
        std::cout << app.help() << std::endl;
        return std::nullopt;
    } catch (const CLI::ParseError &e) {
        app.exit(e);
        return std::nullopt;
    }
    
    if (app.got_subcommand(run)) {
        return RunConfig{filename, watch};
    }
    
    return std::nullopt;
}

}