cmake_minimum_required(VERSION 3.15)
project(cvrpstw)

set(CMAKE_CXX_STANDARD 20)
file(GLOB_RECURSE cvrpstw_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )
set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")



add_library(cvrpstw SHARED ${cvrpstw_SRC})



target_include_directories(cvrpstw PUBLIC ../../core/src/)
target_include_directories(cvrpstw PUBLIC ../vrp/src/)
target_include_directories(cvrpstw PUBLIC ../cvrp/src/)
target_include_directories(cvrpstw PUBLIC ../cvrptw/src/)


target_link_libraries(cvrpstw PUBLIC cvrptw)
target_link_libraries(cvrpstw PUBLIC dl)
target_link_libraries(cvrpstw PUBLIC ${CPLEX_LIBRARIES})