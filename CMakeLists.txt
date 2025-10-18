cmake_minimum_required(VERSION 3.22)
project(cli11_test VERSION 0.1.0 LANGUAGES C CXX)


set(V8_ROOT "/v8" CACHE PATH "Path to v8 repo root")
set(V8_INC "${V8_ROOT}/include")
set(V8_BUILD "x64.release.sample" CACHE STRING "V8 out.gn build dir (e.g. x64.release, x64.release.sample)")
set(V8_OUT "${V8_ROOT}/out.gn/${V8_BUILD}")
set(V8_LIB "${V8_OUT}/obj")


message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Generator: ${CMAKE_GENERATOR}")

if(NOT DEFINED CMAKE_CXX_COMPILER)
  set(CMAKE_C_COMPILER  "/usr/bin/gcc-11"  CACHE FILEPATH "" FORCE)
  set(CMAKE_CXX_COMPILER "/usr/bin/g++-11" CACHE FILEPATH "" FORCE)
endif()


if (NOT EXISTS "${V8_INC}/v8.h")
    message(FATAL_ERROR, "V8 headers not found at ${V8_INC}  (expected v8.h). Check V8_ROOT=${V8_ROOT}")    
endif()

if(NOT EXISTS "${V8_LIB}/libv8_monolith.a")
  message(FATAL_ERROR "V8 libs not found at ${V8_LIB} (expected libv8_monolith.a). Did you build v8_monolith in ${V8_OUT}?")
endif()

# if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/nlohmann/json.hpp")
#   message(FATAL_ERROR "Missing external/nlohmann/json.hpp. Run: ./scripts/bootstrap.sh")
# endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory(external/CLI11)
add_subdirectory(external/json-schema-validator)

add_executable(cli11_test 
  src/main.cpp
  external/json-schema-validator/src/json-uri.cpp
  # external/json-schema-validator/src/json-schema.cpp
)

target_include_directories(cli11_test PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/external/CLI11/include
    include
    ${V8_INC}
)

include(FetchContent)

FetchContent_Declare(efsw
  GIT_REPOSITORY https://github.com/SpartanJ/efsw.git
  GIT_TAG master
)


FetchContent_MakeAvailable(efsw)

target_include_directories(cli11_test PRIVATE
  ${efsw_SOURCE_DIR}/include
)


FetchContent_Declare(p_ranav_glob
  GIT_REPOSITORY https://github.com/p-ranav/glob.git
  GIT_TAG master
)

FetchContent_MakeAvailable(p_ranav_glob)

target_include_directories(cli11_test PRIVATE 
  ${p_ranav_glob_SOURCE_DIR}/single_include
)



target_compile_features(cli11_test PUBLIC cxx_std_20)
target_link_libraries(cli11_test PRIVATE CLI11::CLI11)

target_link_directories(cli11_test PRIVATE ${V8_LIB})
target_link_libraries(cli11_test PRIVATE v8_monolith v8_libbase v8_libplatform dl pthread)



target_link_libraries(cli11_test PRIVATE efsw)
# target_link_libraries(cli11_test PRIVATE p_ranav_glob)


# target_link_libraries(cli11_test PRIVATE nlohmann_json::nlohmann_json)

# link flag to a library that does not exist (yet)
target_link_libraries(cli11_test PRIVATE nlohmann_json_schema_validator)


# set(CMAKE_C_COMPILER  "/usr/bin/gcc-13"  CACHE FILEPATH "" FORCE)
# set(CMAKE_CXX_COMPILER "/usr/bin/g++-13" CACHE FILEPATH "" FORCE)

# https://v8.dev/blog/pointer-compression damn bro
target_compile_definitions(cli11_test PRIVATE V8_COMPRESS_POINTERS V8_ENABLE_SANDBOX ) 
target_compile_options(cli11_test PRIVATE -fno-rtti) 
target_link_options(cli11_test PRIVATE -fuse-ld=lld)


