#pragma once
#include <iostream>
#include <string>
#include <nlohmann/json-schema.hpp>

using json = nlohmann::json;
using nlohmann::json_schema::json_validator;
using PackageJson = json;

// Template interface
template <class T> 
struct JsonSchema {
    static const T schema();
    static T build(const json &json);
};

// Generic validator/loader
template <class T> 
class ConfigLoader {
public:
    explicit ConfigLoader(const json &schema) : schema_(schema) {
        try {
            validator_.set_root_schema(schema);
        } catch (const std::exception &e) {
            std::cerr << "Schema validation failed: " << e.what() << "\n";
        }
    }

    T Parse(const json &json) {
        validator_.validate(json);
        return JsonSchema<T>::build(json);
    }

private:
    json schema_;
    json_validator validator_;
};

// Specialization for PackageJson
template <> 
inline const PackageJson JsonSchema<PackageJson>::schema() {
    static const json package_json_schema = R"({
        "$schema": "https://www.schemastore.org/package.json"
    })"_json;
    return package_json_schema;
}

template <> 
inline PackageJson JsonSchema<PackageJson>::build(const json &json) {
    return json;
}