cmake_minimum_required(VERSION 3.15)
project(cvrptw)

set(CMAKE_CXX_STANDARD 17)
file(GLOB_RECURSE cvrptw_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )
set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")



add_library(cvrptw SHARED ${cvrptw_SRC})



target_include_directories(cvrptw PUBLIC ../../core/src/)
target_include_directories(cvrptw PUBLIC ../vrp/src/)
target_include_directories(cvrptw PUBLIC ../cvrp/src/)


target_link_libraries(cvrptw PUBLIC cvrp)
target_link_libraries(cvrptw PUBLIC dl)
target_link_libraries(cvrptw PUBLIC ${CPLEX_LIBRARIES})