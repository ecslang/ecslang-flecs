add_subdirectory(module)

# Make test executable
add_executable(tests
        ecs.cpp
        qjs.cpp)

target_link_libraries(tests PRIVATE
        flecs
        doctest::main
        quickjs-static)
