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
  explicit ConfigLoader(const json &schema);

  T Parse(const json &json);

private:
  json schema_;
  json_validator validator_;
};

template <> 
struct JsonSchema<PackageJson> {
  static const PackageJson schema();
  static PackageJson build(const json &json);
};