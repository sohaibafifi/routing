cmake_minimum_required(VERSION 3.12)
project(routing)
enable_testing()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_OSX_ARCHITECTURES "x86_64")
file(GLOB_RECURSE example_vrptwtd_SRC
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
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")

link_directories(${CPLEX_STUDIO_DIR}/concert/lib/${CPLEX_STUDIO_ARCH}/static_pic)
link_directories(${CPLEX_STUDIO_DIR}/cplex/lib/${CPLEX_STUDIO_ARCH}/static_pic)
link_directories(${CPLEX_STUDIO_DIR}/cpoptimizer/lib/${CPLEX_STUDIO_ARCH}/static_pic)


add_executable(example_vrptwtd ${example_vrptwtd_SRC})


add_subdirectory(../../routing routing)

target_include_directories(example_vrptwtd PUBLIC ../../core/src)
target_include_directories(example_vrptwtd PUBLIC ../../problems/vrp/src)
target_include_directories(example_vrptwtd PUBLIC ../../problems/cvrp/src)
target_include_directories(example_vrptwtd PUBLIC ../../problems/cvrptw/src)
target_include_directories(example_vrptwtd PUBLIC ../../problems/vrptwtd/src)


target_link_libraries(example_vrptwtd PUBLIC routing)


if (EXISTS "${CPLEX_STUDIO_DIR}/cplex/include/ilcplex/ilocplexi.h")
    target_include_directories(example_vrptwtd SYSTEM PUBLIC ${CPLEX_STUDIO_DIR}/concert/include)
    target_include_directories(example_vrptwtd SYSTEM PUBLIC ${CPLEX_STUDIO_DIR}/cplex/include)
    target_include_directories(example_vrptwtd SYSTEM PUBLIC ${CPLEX_STUDIO_DIR}/cpoptimizer/include)
    target_compile_definitions(example_vrptwtd PUBLIC IL_STD)
    target_compile_definitions(example_vrptwtd PUBLIC CPLEX)
    target_link_libraries(example_vrptwtd PUBLIC dl)
    target_link_libraries(example_vrptwtd PUBLIC cp)
    target_link_libraries(example_vrptwtd PUBLIC ilocplex)
    target_link_libraries(example_vrptwtd PUBLIC cplex)
    target_link_libraries(example_vrptwtd PUBLIC concert)
    target_link_libraries(example_vrptwtd PUBLIC pthread)
endif ()

