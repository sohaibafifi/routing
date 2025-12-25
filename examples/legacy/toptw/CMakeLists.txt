file(GLOB_RECURSE example_toptw_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )


set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")


add_executable(example_toptw ${example_toptw_SRC})



target_include_directories(example_toptw PUBLIC ${PROJECT_SOURCE_DIR}/problems/vrp/src)
target_include_directories(example_toptw PUBLIC ${PROJECT_SOURCE_DIR}/problems/cvrp/src)
target_include_directories(example_toptw PUBLIC ${PROJECT_SOURCE_DIR}/problems/top/src)
target_include_directories(example_toptw PUBLIC ${PROJECT_SOURCE_DIR}/problems/toptw/src)
target_include_directories(example_toptw PUBLIC ${PROJECT_SOURCE_DIR}/core/src)
target_include_directories(example_toptw PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(example_toptw PUBLIC ${PROJECT_SOURCE_DIR}/examples)


target_link_libraries(example_toptw PUBLIC routing)


routing_target_link_dl(example_toptw)
target_link_libraries(example_toptw PUBLIC ${CPLEX_LIBRARIES})
