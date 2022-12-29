cmake_minimum_required(VERSION 3.15)
project(vrptwtd)

set(CMAKE_CXX_STANDARD 20)
file(GLOB_RECURSE vrptwtd_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )
set(CMAKE_VERBOSE_MAKEFILE on)



set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")



add_library(vrptwtd SHARED ${vrptwtd_SRC})



target_include_directories(vrptwtd PUBLIC ../../core/src/)
target_include_directories(vrptwtd PUBLIC ../vrp/src/)
target_include_directories(vrptwtd PUBLIC ../cvrp/src/)
target_include_directories(vrptwtd PUBLIC ../cvrptw/src/)


target_link_libraries(vrptwtd PUBLIC cvrptw)


target_link_libraries(vrptwtd PUBLIC dl)
target_link_libraries(vrptwtd PUBLIC ${CPLEX_LIBRARIES})