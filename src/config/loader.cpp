#pragma once

#include <stdlib.h>
#include <string>
#include <vector>
#include <unordered_set>
#include <initializer_list>

#include <nlohmann/json.hpp>

using json = nlohmann::json;
using nlohmann::json_schema::json_validator;


template <class T>
struct JsonSchema;


namespace detail {
  inline void check_allowed_keys(
    const json& j,
    const std::unordered_set<std::string>& allowed) {
      for (auto it = j.begin(); it!=j.end(); ++it) {
        if (!allowed.contains(it.key())) {
          throw std::runtime_error("Unknown key" + it.key());
        }
      }
    }
}

struct Person {
  std::string name;
  std::vector<std::string> skills;
};

class ConfigSerializer {
  public:
    static Person ParsePackageJson(const json& json) {
      static const std::unordered_set<std::string> allowed{"name", "skills"};

      for (auto it = json.begin(); it != json.end(); it++) {
        if (!allowed.count(it.key())) {
          throw std::runtime_error("Unknown key " + it.key());
        }
      };

      return ValidatePackageJson(json);
    }

  private:
    static Person ValidatePackageJson(const json& json) {
      if (!json.contains("name") || !json["name"].is_string()) {
        throw std::runtime_error("Field 'name' is required and must be string");
      }

      std::string name = json["name"].get<std::string>();
      if (name.empty()) {
        throw std::runtime_error("'name' must be not empty");
      }

      if (!json.contains("skills") || !json["skills"].is_array()) {
        throw std::runtime_error("Skills are required and must be array");
      }

      std::vector<std::string> skills = json["skills"].get<std::vector<std::string>>();
      if (skills.empty()) {
        throw std::runtime_error("'skills' must be not empty");
      }

      if (skills.size() > 256) {
        throw std::runtime_error("too many skills");
      }

      return Person{std::move(name), std::move(skills)};
    }
};