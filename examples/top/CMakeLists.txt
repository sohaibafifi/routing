cmake_minimum_required(VERSION 3.12)
project(example_top)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OSX_ARCHITECTURES "x86_64")
file(GLOB_RECURSE example_top_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )
set(CMAKE_VERBOSE_MAKEFILE on)


set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")


add_executable(example_top ${example_top_SRC})



target_include_directories(example_top PUBLIC ../../problems/vrp/src)
target_include_directories(example_top PUBLIC ../../problems/cvrp/src)
target_include_directories(example_top PUBLIC ../../problems/top/src)
target_include_directories(example_top PUBLIC ../../core/src)


target_link_libraries(example_top PUBLIC routing)

target_link_libraries(example_top PUBLIC dl)
target_link_libraries(example_top PUBLIC ${CPLEX_LIBRARIES})