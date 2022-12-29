cmake_minimum_required(VERSION 3.12)
project(example_cvrptw)
enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OSX_ARCHITECTURES "x86_64")
file(GLOB_RECURSE example_cvrptw_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )
set(CMAKE_VERBOSE_MAKEFILE on)


set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")




add_executable(example_cvrptw ${example_cvrptw_SRC})



target_include_directories(example_cvrptw PUBLIC ../../problems/vrp/src)
target_include_directories(example_cvrptw PUBLIC ../../problems/cvrp/src)
target_include_directories(example_cvrptw PUBLIC ../../problems/cvrptw/src)
target_include_directories(example_cvrptw PUBLIC ../../core/src)


target_link_libraries(example_cvrptw PUBLIC routing)