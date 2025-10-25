#pragma once

#include <string>
#include <vector>
#include <filesystem>


namespace app::glob {

using GlobPattern = std::string;

std::vector<GlobPattern> build_patterns(bool recursive = false);

std::filesystem::path find_package_json();
std::filesystem::path find_package_json(bool recursive);
}