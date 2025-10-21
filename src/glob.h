#pragma once

#include <string>
#include <vector>


namespace app::glob {

using GlobPattern = std::string;

std::vector<GlobPattern> build_patterns(bool recursive = false);

int find_package_json();
int find_package_json(bool recursive);
}