add_subdirectory(cpp)

add_executable(tests_module_cpp
    test.cpp
)
target_link_libraries(tests_module_cpp
    PRIVATE
    tests_module_cpp_lib
    flecs
    doctest::main)
