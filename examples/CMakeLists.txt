cmake_minimum_required(VERSION 3.12)
project(examples)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OSX_ARCHITECTURES "x86_64")

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

find_package(Cplex)
include_directories(SYSTEM ${CPLEX_INCLUDE_DIRS})



add_subdirectory(../routing routing)
set(problems top toptw cvrptw vrptwtd)
foreach (problem ${problems})
    add_subdirectory(${problem} examples/${problem})
endforeach ()


add_custom_target(examples)
add_dependencies(examples ${problems})