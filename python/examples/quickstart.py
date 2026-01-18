#!/usr/bin/env python3
"""
Quickstart Example - Minimal routing problem

This is the simplest possible example to get started with the routing library.
Demonstrates using type-safe enums for attribute names.
"""

import routing
from routing.constants import Attribute


def main():
    # Initialize routing library
    routing.init()

    # Create a problem
    problem = routing.Problem()

    # Add depot at (0, 0) using enum (recommended for type safety)
    depot = problem.add_depot(0)
    depot.add_attribute(Attribute.GEONODE, 0, 0)

    # Add 3 clients with location and demand
    for i, (x, y, demand) in enumerate([(10, 20, 5), (30, 10, 3), (20, 30, 4)], start=1):
        client = problem.add_client(i)
        client.add_attribute(Attribute.GEONODE, x, y)
        client.add_attribute(Attribute.CONSUMER, demand)

    # Add 1 vehicle with capacity 15
    vehicle = problem.add_vehicle(0)
    vehicle.add_attribute(Attribute.STOCK, 15)

    # Solve (attributes are automatically enabled)
    solution = routing.solve(problem, "ga", timeout=10)

    # Print result
    if solution:
        print(f"Cost: {solution.cost:.2f}")
        for i, tour in enumerate(solution.get_tours()):
            print(f"Route {i}: {tour.get_client_ids()}")
    else:
        print("No solution found")


if __name__ == "__main__":
    main()
