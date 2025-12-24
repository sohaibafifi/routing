set(examples
    example_vrp
    example_cvrp
    example_top
    example_toptw
    example_cvrptw
    example_cvrpstw
    example_vrptwtd
    example_tsptw
)
set(problem_examples vrp cvrp top toptw cvrptw cvrpstw vrptwtd tsptw)
foreach (problem ${problem_examples})
    add_subdirectory(${problem})
endforeach ()

add_custom_target(build_examples)
add_dependencies(build_examples ${examples})
