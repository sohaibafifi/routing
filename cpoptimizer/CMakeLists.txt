cmake_minimum_required(VERSION 3.12)
project(cpoptimizer)
enable_testing()

set(GUROBI_DIR "/Library/gurobi1000/macos_universal2/")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OSX_ARCHITECTURES "x86_64")
file(GLOB_RECURSE cpoptimizer_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
find_package(Cplex REQUIRED)

include_directories(${CPLEX_INCLUDE_DIR})
add_executable(cpoptimizer  ${cpoptimizer_SRC})
target_include_directories(cpoptimizer PUBLIC ../examples/libs)

target_link_libraries(cpoptimizer ${CPLEX_LIBRARIES})




