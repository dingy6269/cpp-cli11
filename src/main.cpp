#include <CLI/CLI.hpp>
#include <iostream>


int main(int argc, char** argv) {
    CLI::App app {"My CLI tool"};

    std::string name;
    bool shout = false;

    option = app.add_option("-n,--name", name, "Name")
}