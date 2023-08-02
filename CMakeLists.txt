cmake_minimum_required(VERSION 3.15)
project(ecslang LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# Prepare doctest for other targets to use
add_subdirectory(extern/doctest)
add_subdirectory(extern/flecs)

add_subdirectory(test)
