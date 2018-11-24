cmake_minimum_required(VERSION 3.9)
project(hybrid)

set(CMAKE_CXX_STANDARD 11)
set(CPLEX_STUDIO_DIR /home/afifi/libs/cplex)

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")

link_directories(${CPLEX_STUDIO_DIR}/concert/lib/x86-64_linux/static_pic)
link_directories(${CPLEX_STUDIO_DIR}/cplex/lib/x86-64_linux/static_pic)
link_directories(${CPLEX_STUDIO_DIR}/cpoptimizer/lib/x86-64_linux/static_pic)

file(GLOB_RECURSE hybrid_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )
add_executable(hybrid ${hybrid_SRC})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sign-compare -pedantic")

target_include_directories(hybrid PUBLIC ${CPLEX_STUDIO_DIR}/concert/include)
target_include_directories(hybrid PUBLIC ${CPLEX_STUDIO_DIR}/cplex/include)
target_include_directories(hybrid PUBLIC ${CPLEX_STUDIO_DIR}/cpoptimizer/include)
target_link_libraries(hybrid PUBLIC rt)
target_link_libraries(hybrid PUBLIC dl)
target_link_libraries(hybrid PUBLIC cp)
target_link_libraries(hybrid PUBLIC ilocplex)
target_link_libraries(hybrid PUBLIC cplex)
target_link_libraries(hybrid PUBLIC concert)
target_link_libraries(hybrid PUBLIC pthread)
target_compile_definitions(hybrid PRIVATE _DOUBLE)
target_compile_definitions(hybrid PRIVATE IL_STD)
target_compile_definitions(hybrid PRIVATE MIPSOLVER)
