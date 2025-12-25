# Composable examples (recommended for new development)
add_subdirectory(composable)
add_subdirectory(plugin_demo)

# Legacy examples (using inheritance-based modeling)
add_subdirectory(legacy)

# Build target for all examples
set(examples
    example_composable
    example_plugin_demo
    example_vrp
    example_cvrp
    example_top
    example_toptw
    example_cvrptw
    example_cvrpstw
    example_vrptwtd
    example_tsptw
)

add_custom_target(build_examples)
add_dependencies(build_examples ${examples})
