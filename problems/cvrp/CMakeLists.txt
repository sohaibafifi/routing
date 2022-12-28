cmake_minimum_required(VERSION 3.12)
project(cvrp)

set(CMAKE_CXX_STANDARD 17)
file(GLOB_RECURSE cvrp_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )
set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")


add_library(cvrp SHARED ${cvrp_SRC})

target_include_directories(cvrp PUBLIC ../../core/src/)
target_include_directories(cvrp PUBLIC ../vrp/src/)

target_link_libraries(cvrp PUBLIC vrp)

target_link_libraries(cvrp PUBLIC dl)
target_link_libraries(cvrp PUBLIC ${CPLEX_LIBRARIES})