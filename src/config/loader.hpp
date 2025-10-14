#pragma once

#include <initializer_list>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <nlohmann/json-schema.hpp>

using json = nlohmann::json;
using nlohmann::json_schema::json_validator;
using PackageJson = json;

template <class T> struct JsonSchema;


template <class T> class ConfigLoader {
public:
  explicit ConfigLoader(const json &schema) : schema_(schema) {
    try {
      validator_.set_root_schema(schema);
    } catch (const std::exception &e) {
      std::cerr << "Validation of schema failed, here is why: " << e.what()
                << "\n";
    }
  }

  T Parse(const json &json) {
    auto defaultPatch = validator_.validate(json);

    return JsonSchema<T>::build(json);
  }

private:
  json schema_;
  json_validator validator_;
};

// why like this here
template <> struct JsonSchema<PackageJson> {
  static const PackageJson schema() {
    static json package_json_schema = R"(
  {
     "$schema": "https://www.schemastore.org/package.json"
  }
  )"_json;

    return package_json_schema;
  }

  static PackageJson build(const json &json) {
    return static_cast<PackageJson>(json);
  }
};