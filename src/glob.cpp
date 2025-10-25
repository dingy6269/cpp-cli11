#include "glob.h"

#include <array>
#include <string_view>
#include <iostream>
#include <filesystem>

#include <glob/glob.hpp>

namespace {
constexpr std::array<std::string_view, 1> VALID_CONFIG_FILES{ "package" };
constexpr std::array<std::string_view, 1> VALID_CONFIG_EXTENSIONS{ "json" };
}

namespace app::glob {
std::vector<GlobPattern> build_patterns(bool recursive) {
  std::vector<std::string> out;
  int reserve_size = VALID_CONFIG_FILES.size() * VALID_CONFIG_EXTENSIONS.size();

  out.reserve(reserve_size);

  for (auto file : VALID_CONFIG_FILES) {
    for (auto extension : VALID_CONFIG_EXTENSIONS) {
      std::string pattern;
      if (!recursive) {
        pattern = std::string(file) + "." + std::string(extension);
      } else {
        pattern = "**/" + std::string(file) + "." + std::string(extension);
      };

      out.emplace_back(pattern);
    }
  }

  return out;
}

std::filesystem::path find_package_json() {
    return find_package_json(true);
}

namespace extglob = ::glob;

std::filesystem::path find_package_json(bool recursive) {
  std::vector<::glob::fs::path> packages;
  int found = 0;

  for (const auto &t : extglob::glob(build_patterns(recursive))) {
    std::cout << t << std::endl;
    packages.push_back(t);
  };

  return packages[0];
};
}