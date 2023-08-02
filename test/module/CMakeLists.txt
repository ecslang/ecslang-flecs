add_subdirectory(cpp)

add_executable(tests_module_cpp_static
    test.cpp
)
target_compile_definitions(tests_module_cpp_static PRIVATE MODULE_TEST_STATIC)
target_link_libraries(tests_module_cpp_static
    PRIVATE
    tests_module_cpp_lib_static
    flecs_static
    doctest::doctest)

add_executable(tests_module
        test.cpp
)
target_link_libraries(tests_module
        PRIVATE
        flecs_shared
        doctest::doctest)
add_dependencies(tests_module
    module_simple
)

add_executable(tests_module_cpp_explorer
    explorer.cpp
)
target_link_libraries(tests_module_cpp_explorer
    PRIVATE
    tests_module_cpp_lib_static
    flecs
)
