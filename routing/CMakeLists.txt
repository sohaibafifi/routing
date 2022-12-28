cmake_minimum_required(VERSION 3.12)
project(routing)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OSX_ARCHITECTURES "x86_64")

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")


set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")

find_package(Cplex)
include_directories(SYSTEM ${CPLEX_INCLUDE_DIRS})

add_library(routing SHARED routing.hpp)
set_target_properties(routing PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(routing PUBLIC ../core/src)

foreach (problem vrp cvrp cvrptw vrptwtd top toptw)
    add_subdirectory(../problems/${problem} ${problem})
    target_include_directories(routing PUBLIC ../problems/${problem}/src)
    target_link_libraries(routing PUBLIC ${problem})
endforeach ()