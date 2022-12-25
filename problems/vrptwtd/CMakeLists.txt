cmake_minimum_required(VERSION 3.15)
project(vrptwtd)

set(CMAKE_CXX_STANDARD 17)
file(GLOB_RECURSE vrptwtd_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )
set(CMAKE_VERBOSE_MAKEFILE on)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CPLEX_STUDIO_DIR /Applications/CPLEX_Studio221/)
    set(CPLEX_STUDIO_ARCH x86-64_osx)
else ()
    set(CPLEX_STUDIO_DIR /opt/ibm/ILOG/CPLEX_Studio221)
    set(CPLEX_STUDIO_ARCH x86-64_linux)
endif ()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")


add_subdirectory(../cvrptw cvrptw)

add_library(vrptwtd SHARED ${vrptwtd_SRC})



target_include_directories(vrptwtd PUBLIC ../../core/src/)
target_include_directories(vrptwtd PUBLIC ../vrp/src/)
target_include_directories(vrptwtd PUBLIC ../cvrp/src/)
target_include_directories(vrptwtd PUBLIC ../cvrptw/src/)


target_link_libraries(vrptwtd PUBLIC cvrptw)


if (EXISTS "${CPLEX_STUDIO_DIR}/cplex/include/ilcplex/ilocplexi.h")
    target_include_directories(vrptwtd SYSTEM PUBLIC ${CPLEX_STUDIO_DIR}/concert/include)
    target_include_directories(vrptwtd SYSTEM PUBLIC ${CPLEX_STUDIO_DIR}/cplex/include)
    target_include_directories(vrptwtd SYSTEM PUBLIC ${CPLEX_STUDIO_DIR}/cpoptimizer/include)
    target_compile_definitions(vrptwtd PUBLIC IL_STD)
    target_compile_definitions(vrptwtd PUBLIC CPLEX)
endif ()