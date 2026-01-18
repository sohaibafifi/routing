"""
Routing Library - Python API

A composable, multi-solver VRP framework with plugin-based attributes.

Quick Start:
    >>> import routing
    >>> from routing.constants import Attribute
    >>>
    >>> routing.init()
    >>>
    >>> # Create a problem
    >>> problem = routing.Problem()
    >>>
    >>> # Add depot with attributes (using enums for type safety)
    >>> depot = problem.add_depot(0)
    >>> depot.add_attribute(Attribute.GEONODE, 0, 0)
    >>>
    >>> # Add clients with attributes
    >>> c1 = problem.add_client(1)
    >>> c1.add_attribute(Attribute.GEONODE, 10, 20)
    >>> c1.add_attribute(Attribute.CONSUMER, 5)
    >>>
    >>> # Add vehicle with attributes
    >>> v = problem.add_vehicle(0)
    >>> v.add_attribute(Attribute.STOCK, 100)
    >>>
    >>> # Solve (attributes are automatically enabled)
    >>> solution = routing.solve(problem, "ga", timeout=30)
    >>> print(f"Cost: {solution.cost}")

Discovery:
    >>> # List available solvers
    >>> routing.list_solvers()
    ['ga', 'ma', 'vns', 'pso', 'ls', 'alns', 'mip', 'cp', ...]
    >>>
    >>> # List available attributes
    >>> routing.list_attributes()
    ['GeoNode', 'Consumer', 'Stock', 'Rendezvous', ...]
    >>>
    >>> # Print all available resources
    >>> routing.print_available_resources()
"""

from ._routing_core import (
    # Core classes
    Problem,
    Solution,
    Tour,
    Client,
    Vehicle,
    Depot,
    Entity,
    SolverBase,

    # Functions
    create_solver,
    solve,
    solve_with_callback,
    list_solvers,
    init,

    # Solution helpers
    create_solution,

    # NumPy helpers
    set_distance_matrix,
    has_custom_distances,
    clear_distance_matrix,
    get_custom_distance,
    set_demands_bulk,
    set_locations_bulk,
    set_time_windows_bulk,
    set_service_times_bulk,
    set_vehicle_capacities_bulk,

    # Version
    __version__,
)

# Import attributes submodule
from ._routing_core import attributes

# Import readers
from .problem import load_solomon, load_tsplib, ProblemBuilder
from . import readers

# Import constants and discovery
from .constants import Attribute, Attr, ProblemType, AVAILABLE_ATTRIBUTES
from .discovery import (
    list_attributes,
    get_attribute_info,
    get_problem_types,
    get_solver_info,
    print_available_resources,
)

# Import visualization (optional dependency)
try:
    from .visualization import (
        plot_solution,
        plot_problem,
        plot_gantt,
        plot_convergence,
        create_animation,
        compare_solutions,
    )
    _HAS_VISUALIZATION = True
except ImportError:
    _HAS_VISUALIZATION = False

# Convenience aliases
Solver = SolverBase

__all__ = [
    # Classes
    "Problem",
    "Solution",
    "Tour",
    "Client",
    "Vehicle",
    "Depot",
    "Entity",
    "Solver",
    "SolverBase",
    "ProblemBuilder",

    # Functions
    "create_solver",
    "solve",
    "solve_with_callback",
    "list_solvers",
    "init",

    # Helpers
    "create_solution",

    # NumPy helpers
    "set_distance_matrix",
    "has_custom_distances",
    "clear_distance_matrix",
    "get_custom_distance",
    "set_demands_bulk",
    "set_locations_bulk",
    "set_time_windows_bulk",
    "set_service_times_bulk",
    "set_vehicle_capacities_bulk",

    # Constants and enums
    "Attribute",
    "Attr",
    "ProblemType",
    "AVAILABLE_ATTRIBUTES",

    # Discovery functions
    "list_attributes",
    "get_attribute_info",
    "get_problem_types",
    "get_solver_info",
    "print_available_resources",

    # Submodules
    "attributes",
    "readers",

    # File readers
    "load_solomon",
    "load_tsplib",

    # Visualization (if matplotlib available)
    "plot_solution",
    "plot_problem",
    "plot_gantt",
    "plot_convergence",
    "create_animation",
    "compare_solutions",

    # Metadata
    "__version__",
]
