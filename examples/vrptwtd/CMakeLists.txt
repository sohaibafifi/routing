file(GLOB_RECURSE example_vrptwtd_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )


set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")

add_executable(example_vrptwtd ${example_vrptwtd_SRC})



target_include_directories(example_vrptwtd PUBLIC ../../core/src)
target_include_directories(example_vrptwtd PUBLIC ../../problems/vrp/src)
target_include_directories(example_vrptwtd PUBLIC ../../problems/cvrp/src)
target_include_directories(example_vrptwtd PUBLIC ../../problems/cvrptw/src)
target_include_directories(example_vrptwtd PUBLIC ../../problems/vrptwtd/src)


target_link_libraries(example_vrptwtd PUBLIC vrptwtd)

routing_target_link_dl(example_vrptwtd)
target_link_libraries(example_vrptwtd PUBLIC ${CPLEX_LIBRARIES})
