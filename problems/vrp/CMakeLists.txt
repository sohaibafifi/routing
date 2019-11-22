cmake_minimum_required(VERSION 3.15)
project(vrp)

set(CMAKE_CXX_STANDARD 14)
file(GLOB_RECURSE vrp_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )
set(CMAKE_VERBOSE_MAKEFILE on)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CPLEX_STUDIO_DIR /Applications/CPLEX_Studio129/)
    set(CPLEX_STUDIO_ARCH x86-64_osx)
else ()
    set(CPLEX_STUDIO_DIR /opt/ibm/ILOG/CPLEX_Studio128)
    set(CPLEX_STUDIO_ARCH x86-64_linux)
endif ()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")

add_library(vrp ${vrp_SRC})

target_include_directories(vrp PUBLIC ../../utilities/src/)
target_include_directories(vrp PUBLIC ../../core/src/)


target_include_directories(vrp PUBLIC ${CPLEX_STUDIO_DIR}/concert/include)
target_include_directories(vrp PUBLIC ${CPLEX_STUDIO_DIR}/cplex/include)
target_include_directories(vrp PUBLIC ${CPLEX_STUDIO_DIR}/cpoptimizer/include)
target_compile_definitions(vrp PRIVATE IL_STD)