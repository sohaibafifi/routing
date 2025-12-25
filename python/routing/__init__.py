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

    # Version
    __version__,
)

# Import attributes submodule
from ._routing_core import attributes

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

    # Submodules
    "attributes",

    # Metadata
    "__version__",
]
