file(GLOB_RECURSE cpoptimizer_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )
add_executable(cpoptimizer ${cpoptimizer_SRC})
target_include_directories(cpoptimizer PRIVATE ../examples/libs)
target_link_libraries(cpoptimizer PRIVATE ${CPLEX_LIBRARIES})



