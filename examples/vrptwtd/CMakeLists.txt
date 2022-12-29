cmake_minimum_required(VERSION 3.12)
project(example_vrptwtd)
enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OSX_ARCHITECTURES "x86_64")
file(GLOB_RECURSE example_vrptwtd_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )
set(CMAKE_VERBOSE_MAKEFILE on)


set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")

add_executable(example_vrptwtd ${example_vrptwtd_SRC})



target_include_directories(example_vrptwtd PUBLIC ../../core/src)
target_include_directories(example_vrptwtd PUBLIC ../../problems/vrp/src)
target_include_directories(example_vrptwtd PUBLIC ../../problems/cvrp/src)
target_include_directories(example_vrptwtd PUBLIC ../../problems/cvrptw/src)
target_include_directories(example_vrptwtd PUBLIC ../../problems/vrptwtd/src)


target_link_libraries(example_vrptwtd PUBLIC routing)

target_link_libraries(example_vrptwtd PUBLIC dl)
target_link_libraries(example_vrptwtd PUBLIC ${CPLEX_LIBRARIES})