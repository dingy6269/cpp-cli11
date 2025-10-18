#include <array>
#include <glob/glob.hpp>
#include <string_view>

// temporary name

constexpr auto VALID_CONFIG_FILES =
    std::to_array<std::string_view>({"package"});
constexpr auto VALID_CONFIG_EXTENSIONS =
    std::to_array<std::string_view>({"json"});

using GlobPattern = std::string;

std::vector<GlobPattern> build_patterns(bool recursive = false) {
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

int find_package_json() {
  for (const auto &t : glob::glob(build_patterns())) {
    std::cout << "Found config file at " << t << std::endl;
  };
};
