cmake_minimum_required(VERSION 3.9)
project(hybrid)

set(CMAKE_CXX_STANDARD 11)
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CPLEX_STUDIO_DIR /Applications/CPLEX_Studio129/)
    set(CPLEX_STUDIO_ARCH x86-64_osx)
else()
    set(CPLEX_STUDIO_DIR /opt/ibm/ILOG/CPLEX_Studio128)
    set(CPLEX_STUDIO_ARCH x86-64_linux)
endif()

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")


link_directories(${CPLEX_STUDIO_DIR}/concert/lib/${CPLEX_STUDIO_ARCH}/static_pic)
link_directories(${CPLEX_STUDIO_DIR}/cplex/lib/${CPLEX_STUDIO_ARCH}/static_pic)
link_directories(${CPLEX_STUDIO_DIR}/cpoptimizer/lib/${CPLEX_STUDIO_ARCH}/static_pic)

file(GLOB_RECURSE hybrid_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )
add_executable(hybrid ${hybrid_SRC} src/routines/neighborhoods/Shift.cpp src/routines/neighborhoods/Shift.hpp src/problems/cvrptw/routines/operators/Shift.cpp src/problems/cvrptw/routines/operators/Shift.hpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sign-compare -pedantic")

target_include_directories(hybrid PUBLIC ${CPLEX_STUDIO_DIR}/concert/include)
target_include_directories(hybrid PUBLIC ${CPLEX_STUDIO_DIR}/cplex/include)
target_include_directories(hybrid PUBLIC ${CPLEX_STUDIO_DIR}/cpoptimizer/include)
target_link_libraries(hybrid PUBLIC dl)
target_link_libraries(hybrid PUBLIC cp)
target_link_libraries(hybrid PUBLIC ilocplex)
target_link_libraries(hybrid PUBLIC cplex)
target_link_libraries(hybrid PUBLIC concert)
target_link_libraries(hybrid PUBLIC pthread)
target_compile_definitions(hybrid PRIVATE _DOUBLE)
target_compile_definitions(hybrid PRIVATE IL_STD)
target_compile_definitions(hybrid PRIVATE MIPSOLVER)
