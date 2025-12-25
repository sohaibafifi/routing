file(GLOB_RECURSE example_cvrpstw_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )


set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")


add_executable(example_cvrpstw ${example_cvrpstw_SRC})



target_include_directories(example_cvrpstw PUBLIC ../../problems/vrp/src)
target_include_directories(example_cvrpstw PUBLIC ../../problems/cvrp/src)
target_include_directories(example_cvrpstw PUBLIC ../../problems/cvrptw/src)
target_include_directories(example_cvrpstw PUBLIC ../../problems/cvrpstw/src)
target_include_directories(example_cvrpstw PUBLIC ../../core/src)
target_include_directories(example_cvrpstw PUBLIC ../../)


target_link_libraries(example_cvrpstw PUBLIC routing)


routing_target_link_dl(example_cvrpstw)
target_link_libraries(example_cvrpstw PUBLIC ${CPLEX_LIBRARIES})
