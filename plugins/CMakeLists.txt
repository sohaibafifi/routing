add_library(routing_plugins STATIC PluginBundle.cpp)

target_include_directories(routing_plugins PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(routing_plugins PUBLIC routing)
