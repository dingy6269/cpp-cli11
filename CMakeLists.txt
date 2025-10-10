cmake_minimum_required(VERSION 3.22)
project(cli11_test VERSION 0.1.0 LANGUAGES CXX)

set(V8_ROOT "/v8" CACHE PATH "Path to v8 repo root")
set(V8_INC "${V8_ROOT}/include")
set(V8_BUILD "x64.release.sample" CACHE STRING "V8 out.gn build dir (e.g. x64.release, x64.release.sample)")
set(V8_OUT "${V8_ROOT}/out.gn/${V8_BUILD}")
set(V8_LIB "${V8_OUT}/obj")


if (NOT EXISTS "${V8_INC}/v8.h")
    message(FATAL_ERROR, "V8 headers not found at ${V8_INC}  (expected v8.h). Check V8_ROOT=${V8_ROOT}")    
endif()

if(NOT EXISTS "${V8_LIB}/libv8_monolith.a")
  message(FATAL_ERROR "V8 libs not found at ${V8_LIB} (expected libv8_monolith.a). Did you build v8_monolith in ${V8_OUT}?")
endif()


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_subdirectory(external/CLI11)
GG
add_executable(cli11_test src/main.cpp)

target_include_directories(cli11_test PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/external/CLI11/include
    include
    ${V8_INC}
    ${V8_ROOT}
)

target_compile_features(cli11_test PUBLIC cxx_std_20)
target_link_libraries(cli11_test PRIVATE CLI11::CLI11)

target_link_directories(cli11_test PRIVATE ${V8_LIB})
target_link_libraries(cli11_test PRIVATE v8_monolith v8_libbase v8_libplatform dl pthread)

target_compile_definitions(cli11_test PRIVATE V8_COMPRESS_POINTERS V8_ENABLE_SANDBOX ) 
target_compile_options(cli11_test PRIVATE -fno-rtti) 

target_link_options(cli11_test PRIVATE -fuse-ld=lld)