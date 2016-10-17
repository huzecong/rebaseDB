cmake_minimum_required(VERSION 3.6)
project(rebase)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(${CMAKE_CURRENT_BINARY_DIR})
file(GLOB CPP_SOURCE_FILES
        "src/*.h"
        "src/*.cc"
        )
set(SOURCE_FILES
        ${CPP_SOURCE_FILES}
        )

add_executable(redbase ${SOURCE_FILES})