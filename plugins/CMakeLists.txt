add_library(routing_plugins STATIC PluginBundle.cpp)

target_include_directories(routing_plugins PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(routing_plugins PUBLIC routing)

# Link CPLEX if available
if(CPLEX_FOUND)
    target_link_libraries(routing_plugins PUBLIC ${CPLEX_LIBRARIES})
endif()

# Link HiGHS if available
if(HIGHS_FOUND)
    target_link_libraries(routing_plugins PUBLIC ${HIGHS_LIBRARIES})
endif()

# Link OR-Tools if available (used by CPSolver backend)
if(ORTOOLS_FOUND)
    target_link_libraries(routing_plugins PUBLIC ${ORTOOLS_LIBRARIES})
endif()

if(ROUTING_BUILD_XCSP3)
    add_subdirectory(solvers/XCSP3SolverPlugin)
    target_link_libraries(routing_plugins PUBLIC XCSP3SolverPlugin)
    target_compile_definitions(routing_plugins PUBLIC ROUTING_BUILD_XCSP3)
endif()
