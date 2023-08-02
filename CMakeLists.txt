cmake_minimum_required(VERSION 3.15)
project(ecslang LANGUAGES CXX)

# Prepare doctest for other targets to use
add_subdirectory(extern/doctest)

# Make test executable
add_executable(tests src/main.cpp)
target_compile_features(tests PRIVATE cxx_std_17)
target_link_libraries(tests doctest)
