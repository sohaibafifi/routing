cmake_minimum_required(VERSION 3.12)
project(example_toptw)
enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OSX_ARCHITECTURES "x86_64")
file(GLOB_RECURSE example_toptw_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )
set(CMAKE_VERBOSE_MAKEFILE on)


set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")


add_executable(example_toptw ${example_toptw_SRC})



target_include_directories(example_toptw PUBLIC ../../problems/vrp/src)
target_include_directories(example_toptw PUBLIC ../../problems/cvrp/src)
target_include_directories(example_toptw PUBLIC ../../problems/top/src)
target_include_directories(example_toptw PUBLIC ../../problems/toptw/src)
target_include_directories(example_toptw PUBLIC ../../core/src)


target_link_libraries(example_toptw PUBLIC routing)


target_link_libraries(example_toptw PUBLIC dl)
target_link_libraries(example_toptw PUBLIC ${CPLEX_LIBRARIES})
