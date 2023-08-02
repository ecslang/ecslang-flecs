add_library(doctest INTERFACE)

target_include_directories(doctest INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/doctest/doctest)

add_library(doctest_main STATIC
    main.cpp
)
target_link_libraries(doctest_main PUBLIC doctest)

add_library(doctest::doctest ALIAS doctest)
add_library(doctest::main ALIAS doctest_main)
