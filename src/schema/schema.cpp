#include <nlohmann/json-schema.hpp>
#include <unordered_set>

using nlohmann::json;

static json package_json_schema = R"(
{
    "$schema": "https://www.schemastore.org/package.json"
}
)"_json;

struct Person {
    std::string name;
    std::vector<std::string> skills;
}


class PackageJsonLoader {
  public:
    static json ParsePackageJson(const json& json) {
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