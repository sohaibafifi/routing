add_library(routing_plugins STATIC PluginBundle.cpp)

target_include_directories(routing_plugins PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(routing_plugins PUBLIC routing)

if(ROUTING_BUILD_XCSP3)
    add_subdirectory(solvers/XCSP3SolverPlugin)
    target_link_libraries(routing_plugins PUBLIC XCSP3SolverPlugin)
    target_compile_definitions(routing_plugins PUBLIC ROUTING_BUILD_XCSP3)
endif()
