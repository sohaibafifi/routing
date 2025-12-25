file(GLOB_RECURSE example_cvrp_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )


set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")


add_executable(example_cvrp ${example_cvrp_SRC})



target_include_directories(example_cvrp PUBLIC ../../problems/vrp/src)
target_include_directories(example_cvrp PUBLIC ../../problems/cvrp/src)
target_include_directories(example_cvrp PUBLIC ../../core/src)
target_include_directories(example_cvrp PUBLIC ../../)


target_link_libraries(example_cvrp PUBLIC vrp)
target_link_libraries(example_cvrp PUBLIC cvrp)

routing_target_link_dl(example_cvrp)
target_link_libraries(example_cvrp PUBLIC ${CPLEX_LIBRARIES})
