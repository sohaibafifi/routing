cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")

add_library(routing SHARED routing.hpp)
set_target_properties(routing PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(routing PUBLIC ../core/src)
target_include_directories(routing PUBLIC ../)

foreach (problem tsptw pdvrptw pdvrp toptw top vrptwtd cvrpstw cvrptw cvrp vrp)
    add_subdirectory(../problems/${problem} ${problem})
    target_include_directories(routing PUBLIC ../problems/${problem}/src)
    target_link_libraries(routing PUBLIC ${problem})
endforeach ()
