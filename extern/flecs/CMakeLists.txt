set(LIB_PUBLIC_HEADERS flecs/flecs.h)

file(GLOB_RECURSE INC flecs/include/*.h flecs/include/*.hpp)
file(GLOB_RECURSE SRC flecs/src/*.c)

set(LIB_SOURCE_FILES
        ${SRC}
        ${INC})
set(LIB_ADDON_FLAGS "")

if (FLECS_ADDONS)
    set(ALL_ADDONS "Cpp;Module;System;Pipeline;Timer;Meta;Meta_C;Units;Expr;JSON;Doc;Coredoc;Http;Rest;Parser;Plecs;Rules;Snapshot;Stats;Monitor;Metrics;Log;Journal;App;OS_API_IMPL")

    foreach(ADDON IN LISTS FLECS_ADDONS)
        list(FIND ALL_ADDONS ${ADDON} ADDON_INDEX)
        if(${ADDON_INDEX} EQUAL -1)
            message(FATAL_ERROR "Unknown flecs addon: ${ADDON}")
        endif()
        string(TOUPPER "FLECS_${ADDON}" ADDON_FLAG)
        list(APPEND LIB_ADDON_FLAGS ${ADDON_FLAG})
    endforeach()

    message(STATUS "Compiling flecs with custom set of addons: ${FLECS_ADDONS}")
else()
    message(STATUS "Compiling flecs with all addons")
endif()

function(add_flecs_library TARGET_NAME LIB_TYPE)
    add_library(${TARGET_NAME} ${LIB_TYPE} ${LIB_SOURCE_FILES})
    set_property(TARGET ${TARGET_NAME} PROPERTY C_VISIBILITY_PRESET hidden)

    if (LIB_ADDON_FLAGS)
        target_compile_definitions(${TARGET_NAME} PUBLIC FLECS_CUSTOM_BUILD ${LIB_ADDON_FLAGS})
    endif()

    if(FLECS_PIC)
        set_property(TARGET ${TARGET_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
    endif()

    if (LIB_TYPE STREQUAL "STATIC")
        target_include_directories(${TARGET_NAME} PUBLIC flecs/include)
        target_compile_definitions(${TARGET_NAME} PRIVATE flecs_STATIC)
    else()
        target_include_directories(${TARGET_NAME} PUBLIC flecs/include)
        target_compile_definitions(${TARGET_NAME} PRIVATE flecs_EXPORTS)
    endif()

    if(WIN32)
        target_link_libraries(${TARGET_NAME} PRIVATE wsock32 ws2_32)
    endif()
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_link_libraries(${TARGET_NAME} PRIVATE pthread)
    endif()
endfunction()

add_flecs_library(flecs_static STATIC)
add_library(flecs ALIAS flecs_static)

add_flecs_library(flecs_shared SHARED)
