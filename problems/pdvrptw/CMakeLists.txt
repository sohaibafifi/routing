cmake_minimum_required(VERSION 3.12)
project(pdvrptw)

set(CMAKE_CXX_STANDARD 20)
file(GLOB_RECURSE pdvrptw_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")


add_library(pdvrptw SHARED ${pdvrptw_SRC})

target_include_directories(pdvrptw PUBLIC ../../core/src/)
target_include_directories(pdvrptw PUBLIC ../vrp/src/)
target_include_directories(pdvrptw PUBLIC ../cvrp/src/)
target_include_directories(pdvrptw PUBLIC ../cvrptw/src/)

target_link_libraries(pdvrptw PUBLIC cvrptw)
target_link_libraries(pdvrptw PUBLIC dl)
target_link_libraries(pdvrptw PUBLIC ${CPLEX_LIBRARIES})