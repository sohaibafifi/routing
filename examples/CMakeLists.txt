add_executable(example_cvrptw cvrptw.cpp)
target_include_directories(example_cvrptw PUBLIC
        problems/cvrp/src
        problems/cvrptw/src
    ${PROJECT_SOURCE_DIR}/core/src
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/examples
)
target_link_libraries(example_cvrptw PUBLIC routing_plugins ${CPLEX_LIBRARIES})
routing_target_link_dl(example_cvrptw)

add_executable(example_top top.cpp)
target_include_directories(example_top PUBLIC
        problems/cvrp/src
        problems/top/src
    ${PROJECT_SOURCE_DIR}/core/src
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/examples
)
target_link_libraries(example_top PUBLIC routing_plugins ${CPLEX_LIBRARIES})
routing_target_link_dl(example_top)

set(examples
    example_cvrptw
    example_top
)

add_custom_target(build_examples)
add_dependencies(build_examples ${examples})
