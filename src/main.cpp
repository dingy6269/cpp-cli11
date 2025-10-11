#include "sample.h"
#include <CLI/CLI.hpp>

#include <stdlib.h>
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

using v8::Context;
using v8::EscapableHandleScope;
// using v8::External;
using v8::Function;
using v8::FunctionTemplate;
using v8::Global;
using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::MaybeLocal;
using v8::Name;
// using v8::NamedPropertyHandlerConfiguration;
using v8::NewStringType;
using v8::Object;
using v8::ObjectTemplate;
using v8::PropertyCallbackInfo;
using v8::Script;
using v8::String;
using v8::TryCatch;
using v8::Value;

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




MaybeLocal<String> read_file(Isolate* isolate, const string& filename) {
  FILE* file = std::fopen(filename.c_str(), "rb");

  if (file == NULL) {
    return v8::MaybeLocal<String>();
  }

  std::fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  rewind(file);

  std::unique_ptr<char[]> chars(new char[size + 1]);
  chars.get()[size] = '\0';

  for (size_t i = 0; i < size;) {
    i += std::fread(&chars.get()[i], 1, size -i, file);

    if (std::ferror(file)) {
      std::fclose(file);
      return MaybeLocal<String>();
    }
  }
  std::fclose(file);


  std::cout << chars.get() << std::endl;

  MaybeLocal<String> result = String::NewFromUtf8(isolate,
    chars.get(),
    NewStringType::kNormal,
    size
  );

  return result;
}


int main(int argc, char *argv[]) {
  V8Runtime v8(argv[0]);

  auto config = CliConfig::parse(argc, argv);

  config.verify();

  Isolate::CreateParams create_params;
  create_params.array_buffer_allocator =
  v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  Isolate* isolate = Isolate::New(create_params);
  Isolate::Scope isolate_scope(isolate);
  v8::HandleScope scope(isolate);

  Local<v8::String> source;

  bool source_loaded = read_file(isolate, config.filename()).ToLocal(&source);

  if (!source_loaded) {
    throw std::runtime_error(
      "Error reading " + config.filename()
    );
  };

  return 0;

  // return invoke_v8_sample(source);
}