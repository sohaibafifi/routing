file(GLOB_RECURSE grb_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )
add_executable(grb ${grb_SRC})
target_include_directories(grb PRIVATE ${GUROBI_INCLUDE_DIRS} ../examples/libs)

target_link_libraries(grb optimized ${GUROBI_CXX_LIBRARY} debug ${GUROBI_CXX_DEBUG_LIBRARY})
target_link_libraries(grb ${GUROBI_LIBRARY})
