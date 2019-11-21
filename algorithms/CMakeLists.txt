cmake_minimum_required(VERSION 3.15)
project(algorithms)

set(CMAKE_CXX_STANDARD 14)
file(GLOB_RECURSE algorithms_SRC
        "../utilities/src/**.h"
        "../utilities/src/**.hpp"
        "../utilities/src/**.cpp"
        )
add_library(algorithms ${algorithms_SRC})
set_target_properties(algorithms PROPERTIES LINKER_LANGUAGE CXX)
