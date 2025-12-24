file(GLOB_RECURSE example_vrp_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )


set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")


add_executable(example_vrp ${example_vrp_SRC})



target_include_directories(example_vrp PUBLIC ../../problems/vrp/src)
target_include_directories(example_vrp PUBLIC ../../core/src)


target_link_libraries(example_vrp PUBLIC routing)

routing_target_link_dl(example_vrp)
target_link_libraries(example_vrp PUBLIC ${CPLEX_LIBRARIES})
