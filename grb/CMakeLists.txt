cmake_minimum_required(VERSION 3.12)
project(grb)
enable_testing()

set(GUROBI_DIR "/Library/gurobi1000/macos_universal2/")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OSX_ARCHITECTURES "x86_64")
file(GLOB_RECURSE grb_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
find_package(GUROBI REQUIRED)
include_directories(${GUROBI_INCLUDE_DIRS})
add_executable(grb  ${grb_SRC})
target_include_directories(grb PUBLIC ../examples/libs)

target_link_libraries(grb optimized ${GUROBI_CXX_LIBRARY} debug ${GUROBI_CXX_DEBUG_LIBRARY})
target_link_libraries(grb ${GUROBI_LIBRARY})




