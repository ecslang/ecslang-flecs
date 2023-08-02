cmake_minimum_required(VERSION 3.15)
project(ecslang LANGUAGES C CXX)

# Prepare doctest for other targets to use
add_subdirectory(extern/doctest/doctest)
add_subdirectory(extern/flecs)

# Make test executable
add_executable(tests
        test/ecs.cpp
        test/main.cpp)
target_compile_features(tests PRIVATE cxx_std_17)
target_link_libraries(tests PRIVATE
        doctest
        flecs)
