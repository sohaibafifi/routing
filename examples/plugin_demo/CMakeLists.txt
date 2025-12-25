file(GLOB_RECURSE example_plugin_demo_SRC
    "src/**.h"
    "src/**.hpp"
    "src/**.cpp"
)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")

add_executable(example_plugin_demo ${example_plugin_demo_SRC})

target_link_libraries(example_plugin_demo PUBLIC routing_plugins)

routing_target_link_dl(example_plugin_demo)
target_link_libraries(example_plugin_demo PUBLIC ${CPLEX_LIBRARIES})
