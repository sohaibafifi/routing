cmake_minimum_required(VERSION 3.12)
project(top)

set(CMAKE_CXX_STANDARD 20)
file(GLOB_RECURSE top_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )
set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")


add_library(top SHARED ${top_SRC})

target_include_directories(top PUBLIC ../../core/src/)
target_include_directories(top PUBLIC ../vrp/src/)
target_include_directories(top PUBLIC ../cvrp/src/)

target_link_libraries(top PUBLIC cvrp)
target_link_libraries(top PUBLIC vrp)
target_link_libraries(top PUBLIC dl)
target_link_libraries(top PUBLIC ${CPLEX_LIBRARIES})