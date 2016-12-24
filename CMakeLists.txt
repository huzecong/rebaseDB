cmake_minimum_required(VERSION 3.5)
project(rebase)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -std=c++11")

find_package(FLEX)
find_package(BISON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

BISON_TARGET(Parser src/parse.y ${CMAKE_CURRENT_BINARY_DIR}/y.tab.cc COMPILE_FLAGS -vd)
FLEX_TARGET(Lexer src/scan.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c)
ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)

file(GLOB C_SOURCE_FILES
        "src/*.h"
        "src/*.c"
        )
file(GLOB CPP_SOURCE_FILES
        "src/*.h"
        "src/*.cc"
        "src/*.c"
        )
set(SOURCE_FILES
        ${C_SOURCE_FILES}
        ${CPP_SOURCE_FILES}
        ${BISON_Parser_OUTPUTS}
        ${FLEX_Lexer_OUTPUTS}
        )

add_executable(dbcreate ${SOURCE_FILES} "src/dbcreate.cpp")
add_executable(redbase ${SOURCE_FILES} "src/redbase.cpp")
