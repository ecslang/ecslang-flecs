add_library(flecs STATIC
    flecs/flecs.c
    flecs/flecs.h
)

set_property(TARGET flecs PROPERTY C_STANDARD 99)
set_property(TARGET flecs PROPERTY C_STANDARD_REQUIRED ON)

target_include_directories(flecs PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/flecs)
