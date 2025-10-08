cmake_minimum_required(VERSION 3.22)
project(cli11_test VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_subdirectory(external/CLI11)


add_executable(cli11_test src/main.cpp)
target_link_libraries(cli11_test PRIVATE CLI11::CLI11)
target_include_directories(cli11_test PRIVATE include)
