cmake_minimum_required(VERSION 3.12)
project(routing)
enable_testing()

set(CMAKE_CXX_STANDARD 14)
file(GLOB_RECURSE routing_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )
set(CMAKE_VERBOSE_MAKEFILE on)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CPLEX_STUDIO_DIR /Applications/CPLEX_Studio1210/)
    set(CPLEX_STUDIO_ARCH x86-64_osx)
else ()
    set(CPLEX_STUDIO_DIR /opt/ibm/ILOG/CPLEX_Studio1210)
    set(CPLEX_STUDIO_ARCH x86-64_linux)
endif ()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")

link_directories(${CPLEX_STUDIO_DIR}/concert/lib/${CPLEX_STUDIO_ARCH}/static_pic)
link_directories(${CPLEX_STUDIO_DIR}/cplex/lib/${CPLEX_STUDIO_ARCH}/static_pic)
link_directories(${CPLEX_STUDIO_DIR}/cpoptimizer/lib/${CPLEX_STUDIO_ARCH}/static_pic)

add_executable(routing ${routing_SRC})
add_subdirectory(utilities)
target_include_directories(routing PUBLIC utilities/src/)

add_subdirectory(problems/vrp)
add_subdirectory(problems/cvrp)
add_subdirectory(problems/cvrptw)

target_include_directories(routing PUBLIC problems/vrp/src/)
target_include_directories(routing PUBLIC problems/cvrp/src/)
target_include_directories(routing PUBLIC problems/cvrptw/src/)

target_include_directories(routing PUBLIC core/src/)


target_link_libraries(routing PUBLIC utilities)
target_link_libraries(routing PUBLIC vrp)
target_link_libraries(routing PUBLIC cvrp)
target_link_libraries(routing PUBLIC cvrptw)


if (EXISTS "${CPLEX_STUDIO_DIR}/cplex/include/ilcplex/ilocplexi.h")
    target_include_directories(routing PUBLIC ${CPLEX_STUDIO_DIR}/concert/include)
    target_include_directories(routing PUBLIC ${CPLEX_STUDIO_DIR}/cplex/include)
    target_include_directories(routing PUBLIC ${CPLEX_STUDIO_DIR}/cpoptimizer/include)
    target_compile_definitions(routing PRIVATE IL_STD)
    target_compile_definitions(routing PRIVATE CPLEX)
    target_link_libraries(routing PUBLIC dl)
    target_link_libraries(routing PUBLIC cp)
    target_link_libraries(routing PUBLIC ilocplex)
    target_link_libraries(routing PUBLIC cplex)
    target_link_libraries(routing PUBLIC concert)
    target_link_libraries(routing PUBLIC pthread)
endif ()

add_subdirectory(tests)
