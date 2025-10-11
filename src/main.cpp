#include "sample.h"
#include <CLI/CLI.hpp>

#include <map>
#include <string>

#include "libplatform/libplatform.h"
#include "v8-context.h"
#include "v8-initialization.h"
#include "v8-isolate.h"
#include "v8-local-handle.h"
#include "v8-primitive.h"
#include "v8-script.h"

using std::map;
using std::string;

namespace cli_defaults {
  inline constexpr const char *APP_NAME = "cherry";
  inline constexpr const char *DEFAULT_FILENAME = "index.js";
} // namespace cli_defaults

class CliConfig {
public:
  static CliConfig parse(int argc, char *argv[]) {
    CliConfig cfg;

    map<string, string> options;

    CLI::App app{cli_defaults::APP_NAME};

    auto *opt = app.add_option("-n, --name", cfg.filename_, "file name")
                    ->default_val(cli_defaults::DEFAULT_FILENAME);

    try {
      app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
      throw;
    }

    return cfg;
  }

  void verify() const {
    if (filename_.empty()) {
      throw std::runtime_error("No script was specified. \n");
    }
  }

  const std::string &filename() const { return filename_; }

private:
  std::string filename_ = cli_defaults::DEFAULT_FILENAME;
};

class V8Runtime {
public:
  explicit V8Runtime(const char *argv0) {
    v8::V8::InitializeICUDefaultLocation(argv0);
    v8::V8::InitializeExternalStartupData(argv0);
    platform_ = v8::platform::NewDefaultPlatform();

    v8::V8::InitializePlatform(platform_.get());
    v8::V8::Initialize();
  }

  ~V8Runtime() {
    v8::V8::Dispose();
    v8::V8::DisposePlatform();
  }

private:
  std::unique_ptr<v8::Platform> platform_;

  V8Runtime(const V8Runtime &) = delete;
  V8Runtime &operator=(const V8Runtime &) = delete;
};

// int ParseOptions(int argc, ) {

// }

int main(int argc, char *argv[]) {
  V8Runtime v8(argv[0]);

  auto cfg = CliConfig::parse(argc, argv);

  cfg.verify();

  return invoke_v8_sample(cfg.filename(), argv);
}