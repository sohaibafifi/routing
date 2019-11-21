cmake_minimum_required(VERSION 3.15)
project(utilities)

set(CMAKE_CXX_STANDARD 14)
file(GLOB_RECURSE utilities_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )
add_library(utilities ${utilities_SRC})
set_target_properties(utilities PROPERTIES LINKER_LANGUAGE CXX)
