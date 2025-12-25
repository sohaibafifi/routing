# Legacy examples using inheritance-based modeling
# These examples use the traditional VRP -> CVRP -> CVRPTW hierarchy
# Consider using the composable system for new development

set(legacy_examples vrp cvrp top toptw cvrptw cvrpstw vrptwtd tsptw)

foreach (problem ${legacy_examples})
    add_subdirectory(${problem})
endforeach ()
