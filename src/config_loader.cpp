#include "config_loader.h"

const PackageJson JsonSchema<PackageJson>::schema() {
    static json package_json_schema = R"(
        {
            "$schema": "https://www.schemastore.org/package.json"
        }
    )"_json;
    return package_json_schema;
}

PackageJson JsonSchema<PackageJson>::build(const json &json) {
    return static_cast<PackageJson>(json);
}