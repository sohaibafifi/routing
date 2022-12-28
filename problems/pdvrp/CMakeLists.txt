cmake_minimum_required(VERSION 3.12)
project(pdvrp)

set(CMAKE_CXX_STANDARD 17)
file(GLOB_RECURSE pdvrp_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )
set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")


add_library(pdvrp SHARED ${pdvrp_SRC})

target_include_directories(pdvrp PUBLIC ../../core/src/)
target_include_directories(pdvrp PUBLIC ../vrp/src/)
target_include_directories(pdvrp PUBLIC ../cvrp/src/)

target_link_libraries(pdvrp PUBLIC cvrp)
target_link_libraries(pdvrp PUBLIC dl)
target_link_libraries(pdvrp PUBLIC ${CPLEX_LIBRARIES})