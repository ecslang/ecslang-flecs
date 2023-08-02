add_subdirectory(module)

# Make test executable
add_executable(tests
        ecs.cpp)

target_link_libraries(tests PRIVATE
        flecs
        doctest::main)
