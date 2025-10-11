#include <CLI/CLI.hpp>
#include "sample.h"

int main(int argc, char* argv[]) {
  CLI::App app {"cli11"};

  std::string name;

  CLI::Option* option = app.add_option("-n, --name", name, "Name of the user");

  option->default_val("Anonymous");

  CLI11_PARSE(app, argc, argv);

  if (name.empty()) {
    std::cout << "No name provided. \n";
    return 1;
  }

  return invoke_v8_sample(name, argv);
}