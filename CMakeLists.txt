cmake_minimum_required(VERSION 3.10)
project(trepl C)

set(CMAKE_C_STANDARD 11)

find_package(BISON 3.5.0)
find_package(FLEX 2.6.4)
bison_target(
        Parser ${CMAKE_SOURCE_DIR}/src/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.c
)
flex_target(
        Scanner ${CMAKE_SOURCE_DIR}/src/scanner.l ${CMAKE_CURRENT_BINARY_DIR}/scanner.c
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/scanner.h
)
add_flex_bison_dependency(Scanner Parser)

include_directories(${CMAKE_CURRENT_BINARY_DIR} include)

file(GLOB_RECURSE SRC "src/*.c")

add_executable(tlang
        ${BISON_Parser_OUTPUTS}
        ${FLEX_Scanner_OUTPUTS}
        ${SRC})

if (MSVC)
    target_compile_options(tlang PRIVATE /W4)
else ()
    target_compile_options(tlang PRIVATE -Wall -Wextra -pedantic)
endif ()