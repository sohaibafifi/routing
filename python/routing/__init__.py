"""
Routing Library - Python API

A composable, multi-solver VRP framework.

Example:
    >>> import routing
    >>>
    >>> # Create a problem
    >>> problem = routing.Problem()
    >>>
    >>> # Add depot
    >>> depot = problem.add_depot(0)
    >>> routing.set_depot_location(depot, 0, 0)
    >>>
    >>> # Add clients
    >>> c1 = problem.add_client(1)
    >>> routing.set_client_location(c1, 10, 20)
    >>> routing.set_client_demand(c1, 5)
    >>>
    >>> # Add vehicle
    >>> v = problem.add_vehicle(0)
    >>> routing.set_vehicle_capacity(v, 100)
    >>>
    >>> # Solve
    >>> solution = routing.solve(problem, "ga", timeout=30)
    >>> print(f"Cost: {solution.cost}")
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

    # Entity helpers
    set_client_location,
    set_client_demand,
    set_client_time_window,
    set_client_service_time,
    set_vehicle_capacity,
    set_depot_location,
    set_depot_time_window,
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
from .problem import load_solomon, load_tsplib
from . import readers

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

    # Functions
    "create_solver",
    "solve",
    "solve_with_callback",
    "list_solvers",
    "init",

    # Helpers
    "set_client_location",
    "set_client_demand",
    "set_client_time_window",
    "set_client_service_time",
    "set_vehicle_capacity",
    "set_depot_location",
    "set_depot_time_window",
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

    # Submodules
    "attributes",
    "readers",

    # File readers
    "load_solomon",
    "load_tsplib",

    # Metadata
    "__version__",
]
