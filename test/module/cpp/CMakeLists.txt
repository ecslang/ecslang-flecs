add_library(tests_module_cpp_lib_static
    STATIC
    simple.cpp
    simple.h
)
target_compile_definitions(tests_module_cpp_lib_static
    PRIVATE
    MODULE_BUILD
)
target_link_libraries(tests_module_cpp_lib_static
    PRIVATE
    flecs
)

add_library(module_simple
        SHARED
        simple.cpp
        simple.h
)
target_compile_definitions(module_simple PUBLIC MODULE_SHARED)
target_compile_definitions(module_simple PRIVATE MODULE_BUILD)
target_link_libraries(module_simple PUBLIC flecs_shared)
set_target_properties(module_simple
        PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
)
