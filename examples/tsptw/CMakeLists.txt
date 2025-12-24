file(GLOB_RECURSE example_tsptw_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )


set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")

add_executable(example_tsptw ${example_tsptw_SRC})



target_include_directories(example_tsptw PUBLIC ../../core/src)
target_include_directories(example_tsptw PUBLIC ../../problems/vrp/src)
target_include_directories(example_tsptw PUBLIC ../../problems/cvrp/src)
target_include_directories(example_tsptw PUBLIC ../../problems/cvrptw/src)
target_include_directories(example_tsptw PUBLIC ../../problems/tsptw/src)


target_link_libraries(example_tsptw PUBLIC tsptw)

routing_target_link_dl(example_tsptw)
target_link_libraries(example_tsptw PUBLIC ${CPLEX_LIBRARIES})
