file(GLOB_RECURSE example_cvrptw_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )


set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")




add_executable(example_cvrptw ${example_cvrptw_SRC})



target_include_directories(example_cvrptw PUBLIC ../../problems/vrp/src)
target_include_directories(example_cvrptw PUBLIC ../../problems/cvrp/src)
target_include_directories(example_cvrptw PUBLIC ../../problems/cvrptw/src)
target_include_directories(example_cvrptw PUBLIC ../../core/src)
target_include_directories(example_cvrptw PUBLIC ../../)



target_link_libraries(example_cvrptw PUBLIC vrp)
target_link_libraries(example_cvrptw PUBLIC cvrp)
target_link_libraries(example_cvrptw PUBLIC cvrptw)

routing_target_link_dl(example_cvrptw)
target_link_libraries(example_cvrptw PUBLIC ${CPLEX_LIBRARIES})
