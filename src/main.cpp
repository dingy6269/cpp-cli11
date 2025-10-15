#include "sample.h"
#include <CLI/CLI.hpp>

#include <map>
#include <stdlib.h>
#include <string>

#include "libplatform/libplatform.h"
#include "v8-context.h"
#include "v8-initialization.h"
#include "v8-isolate.h"
#include "v8-local-handle.h"
#include "v8-primitive.h"
#include "v8-script.h"

#include "v8-array-buffer.h"
#include "v8-context.h"
#include "v8-exception.h"
#include "v8-external.h"
#include "v8-function.h"
#include "v8-initialization.h"
#include "v8-isolate.h"
#include "v8-local-handle.h"
#include "v8-object.h"
#include "v8-persistent-handle.h"
#include "v8-primitive.h"
#include "v8-script.h"
#include "v8-snapshot.h"
#include "v8-template.h"
#include "v8-value.h"


#include "config/loader.hpp"


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


using json = nlohmann::json;





class V8Bridge {
public:
  virtual ~V8Bridge() {}

  virtual bool Initialize() = 0;
};

class V8BridgeProcessor : public V8Bridge {
public:
  V8BridgeProcessor(Isolate *isolate, Local<String> script)
      : isolate_(isolate) {
    script_.Reset(isolate_, script);
  }
  virtual ~V8BridgeProcessor() {
    if (!context_.IsEmpty()) {
      context_.Reset();
    }
  };

  virtual bool Initialize();

  Isolate *GetIsolate() { return isolate_; }

  // Methods
  static void Log(const char *event);

private:
  bool ExecuteScript(Local<String> script);

  Isolate *isolate_;
  Global<String> script_;
  Global<Context> context_;
};

static void LogCallback(const v8::FunctionCallbackInfo<v8::Value> &info) {
  // Checking the number of potential arguments.
  if (info.Length() < 1)
    return;

  Isolate *isolate = info.GetIsolate();
  HandleScope scope(isolate);
  Local<Value> arg = info[0];
  String::Utf8Value value(isolate, arg);

  V8BridgeProcessor::Log(*value);
}

bool V8BridgeProcessor::Initialize() {
  HandleScope handle_scope(GetIsolate());

  Local<ObjectTemplate> global = ObjectTemplate::New(GetIsolate());
  global->Set(GetIsolate(), "log",
              FunctionTemplate::New(GetIsolate(), LogCallback));

  Local<v8::Context> context = Context::New(GetIsolate(), nullptr, global);
  context_.Reset(GetIsolate(), context);
  Local<String> script_local = script_.Get(GetIsolate());

  v8::Context::Scope context_scope(context);

  if (!ExecuteScript(script_local)) {
    return false;
  }

  return true;
}

bool V8BridgeProcessor::ExecuteScript(Local<String> script) {
  HandleScope handle_scope(GetIsolate());

  TryCatch try_catch(GetIsolate());

  Local<Context> context(GetIsolate()->GetCurrentContext());
  Local<Script> compiled_script;

  if (!Script::Compile(context, script).ToLocal(&compiled_script)) {
    String::Utf8Value error(GetIsolate(), try_catch.Exception());
    Log(*error);

    return false;
  }

  Local<Value> result;
  if (!compiled_script->Run(context).ToLocal(&result)) {
    String::Utf8Value error(GetIsolate(), try_catch.Exception());
    Log(*error);
    return false;
  }

  return true;
}

void V8BridgeProcessor::Log(const char *event) {
  printf("Logged: %s\n", event);
}

namespace cli_defaults {
inline constexpr const char *APP_NAME = "cherry";
inline constexpr const char *DEFAULT_FILENAME = "index.js";
} // namespace cli_defaults

class CliConfig {
public:
  static std::optional<std::string> parse(int argc, char *argv[]) {
    CLI::App app{cli_defaults::APP_NAME};

    auto run = app.add_subcommand("run", "run file");
    std::string filename;

    auto* opt = run->add_option("-n, --name", filename, "file name")
                    ->default_val(cli_defaults::DEFAULT_FILENAME);

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
      return filename;
    }
  
    return std::nullopt;
  }
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

MaybeLocal<String> read_file(Isolate *isolate, const string &filename) {
  FILE *file = std::fopen(filename.c_str(), "rb");

  if (file == NULL) {
    return v8::MaybeLocal<String>();
  }

  std::fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  rewind(file);

  std::unique_ptr<char[]> chars(new char[size + 1]);
  chars.get()[size] = '\0';

  for (size_t i = 0; i < size;) {
    i += std::fread(&chars.get()[i], 1, size - i, file);

    if (std::ferror(file)) {
      std::fclose(file);
      return MaybeLocal<String>();
    }
  }
  std::fclose(file);

  MaybeLocal<String> result =
      String::NewFromUtf8(isolate, chars.get(), NewStringType::kNormal, size);

  return result;
}

int main(int argc, char *argv[]) {
  const json& schema = JsonSchema<PackageJson>::schema();

  ConfigLoader<PackageJson> config_loader(schema);
  
  std::ifstream f("package.json");
  json data = json::parse(f);

  auto patch = config_loader.Parse(data);

  std::cout << "Here is your patch" << patch.dump(2) << std::endl;

  V8Runtime v8(argv[0]);

  auto filename = CliConfig::parse(argc, argv);

  if (!filename) {
    return 0;
  }

  Isolate::CreateParams create_params;
  create_params.array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  Isolate *isolate = Isolate::New(create_params);
  Isolate::Scope isolate_scope(isolate);
  v8::HandleScope scope(isolate);

  Local<v8::String> source;

  bool source_loaded = read_file(isolate, *filename).ToLocal(&source);

  if (!source_loaded) {
    throw std::runtime_error("Error reading " + *filename);
  };

  V8BridgeProcessor bridge(isolate, source);

  if (!bridge.Initialize()) {
    fprintf(stderr, "Error initializing processor. \n");
    return 1;
  }

  

  return 0;
}