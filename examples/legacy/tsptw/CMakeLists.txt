file(GLOB_RECURSE example_tsptw_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )


set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")

add_executable(example_tsptw ${example_tsptw_SRC})



target_include_directories(example_tsptw PUBLIC ${PROJECT_SOURCE_DIR}/core/src)
target_include_directories(example_tsptw PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(example_tsptw PUBLIC ${PROJECT_SOURCE_DIR}/examples)
target_include_directories(example_tsptw PUBLIC ${PROJECT_SOURCE_DIR}/problems/vrp/src)
target_include_directories(example_tsptw PUBLIC ${PROJECT_SOURCE_DIR}/problems/cvrp/src)
target_include_directories(example_tsptw PUBLIC ${PROJECT_SOURCE_DIR}/problems/cvrptw/src)
target_include_directories(example_tsptw PUBLIC ${PROJECT_SOURCE_DIR}/problems/tsptw/src)


target_link_libraries(example_tsptw PUBLIC tsptw)

routing_target_link_dl(example_tsptw)
target_link_libraries(example_tsptw PUBLIC ${CPLEX_LIBRARIES})
