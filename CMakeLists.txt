cmake_minimum_required(VERSION 3.6)
project(rebase)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-deprecated-register")

find_package(FLEX)
find_package(BISON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

FLEX_TARGET(Lexer src/scan.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c)
BISON_TARGET(Parser src/parse.y ${CMAKE_CURRENT_BINARY_DIR}/y.tab.cc COMPILE_FLAGS -vd)
ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)

file(GLOB C_SOURCE_FILES
        "src/*.h"
        "src/*.c"
        )
file(GLOB CPP_SOURCE_FILES
        "src/*.h"
        "src/*.hh"
        "src/*.cc"
        "src/*.c"
        )
set(SOURCE_FILES
        ${C_SOURCE_FILES}
        ${CPP_SOURCE_FILES}
        ${FLEX_Lexer_OUTPUTS}
        ${BISON_Parser_OUTPUTS}
        )

add_executable(redbase ${SOURCE_FILES})