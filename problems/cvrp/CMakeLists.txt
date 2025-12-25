cmake_minimum_required(VERSION 3.12)
project(cvrp)

set(CMAKE_CXX_STANDARD 20)
file(GLOB_RECURSE cvrp_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )
set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")


add_library(cvrp SHARED ${cvrp_SRC})

target_include_directories(cvrp PUBLIC ../../core/src/)
target_include_directories(cvrp PUBLIC ../../)
target_include_directories(cvrp PUBLIC ../vrp/src/)

target_link_libraries(cvrp PUBLIC vrp)

routing_target_link_dl(cvrp)
target_link_libraries(cvrp PUBLIC ${CPLEX_LIBRARIES})
