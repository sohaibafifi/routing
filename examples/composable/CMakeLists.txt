file(GLOB_RECURSE example_composable_SRC
    "src/**.h"
    "src/**.hpp"
    "src/**.cpp"
)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")

add_executable(example_composable ${example_composable_SRC})

target_include_directories(example_composable PUBLIC ../../core/src)
target_include_directories(example_composable PUBLIC ../../problems/vrp/src)

target_link_libraries(example_composable PUBLIC vrp)

routing_target_link_dl(example_composable)
target_link_libraries(example_composable PUBLIC ${CPLEX_LIBRARIES})
