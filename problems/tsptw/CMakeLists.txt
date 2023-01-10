cmake_minimum_required(VERSION 3.15)
project(tsptw)

set(CMAKE_CXX_STANDARD 20)
file(GLOB_RECURSE tsptw_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )
set(CMAKE_VERBOSE_MAKEFILE on)



set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")



add_library(tsptw SHARED ${tsptw_SRC})



target_include_directories(tsptw PUBLIC ../../core/src/)
target_include_directories(tsptw PUBLIC ../vrp/src/)
target_include_directories(tsptw PUBLIC ../cvrp/src/)
target_include_directories(tsptw PUBLIC ../cvrptw/src/)


target_link_libraries(tsptw PUBLIC cvrptw)
target_link_libraries(tsptw PUBLIC cvrp)
target_link_libraries(tsptw PUBLIC vrp)


target_link_libraries(tsptw PUBLIC dl)
target_link_libraries(tsptw PUBLIC ${CPLEX_LIBRARIES})