cmake_minimum_required(VERSION 3.12)
project(vrp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")
set(CMAKE_VERBOSE_MAKEFILE on)


file(GLOB_RECURSE vrp_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )

add_library(vrp SHARED ${vrp_SRC})


target_include_directories(vrp PUBLIC ../../core/src/)

target_link_libraries(vrp PUBLIC dl)
target_link_libraries(vrp PUBLIC ${CPLEX_LIBRARIES})
