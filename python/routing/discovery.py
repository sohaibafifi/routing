"""
Discovery functions for available solvers, attributes, and plugins.

This module provides functions to query what's available in the routing library.
"""

from typing import List, Dict, Any
from .constants import AVAILABLE_ATTRIBUTES


def list_attributes() -> List[str]:
    """
    Get list of all available attribute types.

    Returns:
        List of attribute names that can be used with add_attribute()

    Example:
        >>> import routing
        >>> routing.init()
        >>> attrs = routing.list_attributes()
        >>> print(attrs)
        ['GeoNode', 'Consumer', 'Stock', 'Rendezvous', 'ServiceQuery', ...]
    """
    return AVAILABLE_ATTRIBUTES.copy()


def get_attribute_info() -> Dict[str, Dict[str, Any]]:
    """
    Get detailed information about each attribute.

    Returns:
        Dictionary mapping attribute names to their metadata

    Example:
        >>> info = routing.get_attribute_info()
        >>> print(info['GeoNode']['description'])
        'Coordinates (x, y) for distance calculation'
        >>> print(info['GeoNode']['parameters'])
        ['x: float', 'y: float']
    """
    return {
        "GeoNode": {
            "description": "Coordinates (x, y) for distance calculation",
            "parameters": ["x: float", "y: float"],
            "entities": ["Client", "Depot"],
            "use_cases": ["TSP", "VRP", "CVRP", "VRPTW", "CVRPTW", "TOP", "PDVRP"],
        },
        "Consumer": {
            "description": "Demand at a client node",
            "parameters": ["demand: int"],
            "entities": ["Client"],
            "use_cases": ["CVRP", "CVRPTW", "PDVRP"],
        },
        "Stock": {
            "description": "Vehicle capacity",
            "parameters": ["capacity: int"],
            "entities": ["Vehicle"],
            "use_cases": ["CVRP", "CVRPTW", "PDVRP", "TOP"],
        },
        "Rendezvous": {
            "description": "Time window bounds (open, close)",
            "parameters": ["open: float", "close: float"],
            "entities": ["Client", "Depot"],
            "use_cases": ["VRPTW", "CVRPTW", "TOP"],
        },
        "ServiceQuery": {
            "description": "Service time duration",
            "parameters": ["service_time: float"],
            "entities": ["Client"],
            "use_cases": ["VRPTW", "CVRPTW"],
        },
        "Profiter": {
            "description": "Profit value for orienteering problems",
            "parameters": ["profit: float"],
            "entities": ["Client"],
            "use_cases": ["TOP"],
        },
        "Pickup": {
            "description": "Pickup demand",
            "parameters": ["demand: int"],
            "entities": ["Client"],
            "use_cases": ["PDVRP"],
        },
        "Delivery": {
            "description": "Delivery demand",
            "parameters": ["demand: int"],
            "entities": ["Client"],
            "use_cases": ["PDVRP"],
        },
        "SoftTimeWindows": {
            "description": "Soft time window penalties",
            "parameters": ["wait_penalty: float", "delay_penalty: float"],
            "entities": ["Depot"],
            "use_cases": ["CVRPSTW"],
        },
        "Synced": {
            "description": "Temporal synchronization constraints",
            "parameters": [],
            "entities": ["Client"],
            "use_cases": ["VRPTWTD"],
        },
    }


def get_problem_types() -> Dict[str, List[str]]:
    """
    Get attribute requirements for common problem types.

    Returns:
        Dictionary mapping problem type names to required attributes

    Example:
        >>> types = routing.get_problem_types()
        >>> print(types['CVRPTW'])
        ['GeoNode', 'Consumer', 'Stock', 'Rendezvous', 'ServiceQuery']
    """
    return {
        "TSP": ["GeoNode"],
        "VRP": ["GeoNode"],
        "CVRP": ["GeoNode", "Consumer", "Stock"],
        "VRPTW": ["GeoNode", "Rendezvous", "ServiceQuery"],
        "CVRPTW": ["GeoNode", "Consumer", "Stock", "Rendezvous", "ServiceQuery"],
        "PDVRP": ["GeoNode", "Pickup", "Delivery", "Stock"],
        "TOP": ["GeoNode", "Profiter", "Rendezvous"],
    }


def get_solver_info(solver_name: str = None) -> Dict[str, Any]:
    """
    Get information about available solvers.

    Args:
        solver_name: Optional solver name to get specific info. If None, returns all.

    Returns:
        Dictionary with solver information

    Example:
        >>> info = routing.get_solver_info('ga')
        >>> print(info['description'])
        'Genetic Algorithm - metaheuristic optimization'
        >>> print(info['parameters'])
        {'iterMax': 'int', 'feasibleOnly': 'bool', ...}
    """
    solver_db = {
        "ga": {
            "name": "Genetic Algorithm",
            "description": "Metaheuristic optimization using evolutionary principles",
            "type": "metaheuristic",
            "parameters": {
                "iterMax": {"type": "int", "description": "Maximum iterations"},
                "feasibleOnly": {"type": "bool", "description": "Only generate feasible solutions"},
                "infeasiblePenalty": {"type": "float", "description": "Penalty for constraint violations"},
                "unservedPenalty": {"type": "float", "description": "Penalty per unserved client"},
            },
            "suitable_for": ["CVRP", "CVRPTW", "VRP", "VRPTW"],
        },
        "ma": {
            "name": "Memetic Algorithm",
            "description": "Genetic algorithm with local search",
            "type": "metaheuristic",
            "parameters": {
                "iterMax": {"type": "int", "description": "Maximum iterations"},
                "feasibleOnly": {"type": "bool", "description": "Only generate feasible solutions"},
                "infeasiblePenalty": {"type": "float", "description": "Penalty for constraint violations"},
            },
            "suitable_for": ["CVRP", "CVRPTW", "VRP"],
        },
        "vns": {
            "name": "Variable Neighborhood Search",
            "description": "Local search with systematic neighborhood changes",
            "type": "metaheuristic",
            "parameters": {
                "iterMax": {"type": "int", "description": "Maximum iterations"},
            },
            "suitable_for": ["CVRP", "CVRPTW", "VRP"],
        },
        "pso": {
            "name": "Particle Swarm Optimization",
            "description": "Swarm intelligence optimization",
            "type": "metaheuristic",
            "parameters": {
                "iterMax": {"type": "int", "description": "Maximum iterations"},
            },
            "suitable_for": ["CVRP", "VRP"],
        },
        "ls": {
            "name": "Local Search",
            "description": "Iterative improvement from initial solution",
            "type": "local_search",
            "parameters": {
                "iterMax": {"type": "int", "description": "Maximum iterations"},
            },
            "suitable_for": ["CVRP", "VRP"],
        },
        "alns": {
            "name": "Adaptive Large Neighborhood Search",
            "description": "Destroy and repair with adaptive operator selection",
            "type": "metaheuristic",
            "parameters": {
                "iterMax": {"type": "int", "description": "Maximum iterations"},
                "reactionFactor": {"type": "float", "description": "Weight update reaction (0-1)"},
                "decayFactor": {"type": "float", "description": "Weight decay per segment (0-1)"},
                "temperature": {"type": "float", "description": "Initial temperature for SA"},
                "coolingRate": {"type": "float", "description": "Temperature cooling rate"},
                "segmentSize": {"type": "int", "description": "Iterations between weight updates"},
                "minTemperature": {"type": "float", "description": "Minimum acceptance temperature"},
            },
            "suitable_for": ["CVRP", "CVRPTW", "PDVRP"],
        },
        "mip": {
            "name": "Mixed Integer Programming",
            "description": "Exact optimization using MIP solvers (CPLEX/HiGHS)",
            "type": "exact",
            "parameters": {},
            "suitable_for": ["CVRP", "CVRPTW", "TSP"],
        },
        "cp": {
            "name": "Constraint Programming",
            "description": "Exact optimization using CP solvers (OR-Tools/CPLEX)",
            "type": "exact",
            "parameters": {},
            "suitable_for": ["CVRP", "CVRPTW", "VRPTW"],
        },
    }

    if solver_name:
        return solver_db.get(solver_name, {})
    return solver_db


def print_available_resources():
    """
    Print a formatted summary of all available resources.

    Example:
        >>> import routing
        >>> routing.init()
        >>> routing.print_available_resources()
    """
    print("=" * 60)
    print("Routing Library - Available Resources")
    print("=" * 60)

    # Solvers
    try:
        import routing
        solvers = routing.list_solvers()
        print(f"\n📊 Solvers ({len(solvers)}):")
        for solver in sorted(solvers):
            info = get_solver_info(solver)
            if info:
                print(f"  • {solver:12} - {info.get('name', 'Unknown')}")
            else:
                print(f"  • {solver}")
    except Exception as e:
        print(f"\n📊 Solvers: Unable to list ({e})")

    # Attributes
    print(f"\n🏷️  Attributes ({len(AVAILABLE_ATTRIBUTES)}):")
    for attr in AVAILABLE_ATTRIBUTES:
        info = get_attribute_info().get(attr, {})
        desc = info.get('description', '')
        print(f"  • {attr:18} - {desc}")

    # Problem Types
    print(f"\n📦 Problem Types:")
    for ptype, attrs in get_problem_types().items():
        print(f"  • {ptype:10} - {', '.join(attrs)}")

    print("=" * 60)


__all__ = [
    'list_attributes',
    'get_attribute_info',
    'get_problem_types',
    'get_solver_info',
    'print_available_resources',
]
