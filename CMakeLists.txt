cmake_minimum_required(VERSION 3.12)
project(routing)
enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OSX_ARCHITECTURES "x86_64")

set(CMAKE_VERBOSE_MAKEFILE on)


add_subdirectory(examples examples)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(GUROBI)
if(GUROBI_LIBRARY)
    message(STATUS "GUROBI_LIBRARY: ${GUROBI_LIBRARY}")
    add_subdirectory(grb grb)
endif()
add_subdirectory(tests tests)