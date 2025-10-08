#include <CLI/CLI.hpp>
#include <iostream>


int main(int argc, char* argv[]) {
    CLI::App app {"My CLI tool"};

    std::string name;
    bool shout = false;

    CLI::Option* option = app.add_option("-n,--name", name, "Name of the user");

    option->default_val("Anonymous");

    app.add_flag("-s, --shout", shout, "Uppercase output");

    
    std::cout << name << std::endl;

    CLI11_PARSE(app, argc, argv);


    std::cout << name << std::endl;

    if (name.empty()) {
        std::cout << "No name provided. \n";
        return 1;
    }

    if (shout) {
        for (auto& c: name) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
    }

    std::cout << "Hello, " << name << "!\n";
    return 0;
}