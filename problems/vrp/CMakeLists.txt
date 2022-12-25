cmake_minimum_required(VERSION 3.12)
project(vrp)

set(CMAKE_CXX_STANDARD 14)
file(GLOB_RECURSE vrp_SRC
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

add_library(vrp SHARED ${vrp_SRC})


target_include_directories(vrp PUBLIC ../../core/src/)

if (EXISTS "${CPLEX_STUDIO_DIR}/cplex/include/ilcplex/ilocplexi.h")
    target_include_directories(vrp SYSTEM PUBLIC ${CPLEX_STUDIO_DIR}/concert/include)
    target_include_directories(vrp SYSTEM PUBLIC ${CPLEX_STUDIO_DIR}/cplex/include)
    target_include_directories(vrp SYSTEM PUBLIC ${CPLEX_STUDIO_DIR}/cpoptimizer/include)
    target_compile_definitions(vrp PUBLIC IL_STD)
    target_compile_definitions(vrp PUBLIC CPLEX)
    target_link_libraries(vrp PUBLIC dl)
    target_link_libraries(vrp PUBLIC cp)
    target_link_libraries(vrp PUBLIC ilocplex)
    target_link_libraries(vrp PUBLIC cplex)
    target_link_libraries(vrp PUBLIC concert)
    target_link_libraries(vrp PUBLIC pthread)
endif ()