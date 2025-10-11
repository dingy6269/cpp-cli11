#include <CLI/CLI.hpp>
#include "sample.h"

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


struct CliConfig {
  std::string filename;
};

class V8Runtime {
  public:
  explicit V8Runtime(const char* argv0) {
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

  V8Runtime(const V8Runtime&) = delete;
  V8Runtime& operator=(const V8Runtime&) = delete;
};


// int ParseOptions(int argc, ) {

// }


namespace cli_defaults {
  inline constexpr const char* APP_NAME = "cherry";
  inline constexpr const char* DEFAULT_FILENAME = "index.js";
}

CliConfig parse_cli(int argc, char* argv[])  {
  map<string, string> options;

  CliConfig cfg;
  CLI::App app {cli_defaults::APP_NAME};

  auto* opt = app.add_option("-n, --name", cfg.filename, "file name")
  -> default_val(cli_defaults::DEFAULT_FILENAME);
  
  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError& e) {
    throw;
  }
  
  return cfg;
}


void verify_config(CliConfig cfg) {
  if (cfg.filename.empty()) {
    std::cout << "No filename provided." << std::endl;
    return;
  };
}

int main(int argc, char* argv[]) {
  V8Runtime v8(argv[0]);
  auto cfg = parse_cli(argc, argv);

  verify_config(cfg);

  return invoke_v8_sample(cfg.filename, argv);
}