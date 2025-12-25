#!/usr/bin/env python3
"""
Quickstart Example - Minimal routing problem

This is the simplest possible example to get started with the routing library.
"""

import routing


def main():
    # Create a problem
    problem = routing.Problem()

    # Add depot at (0, 0)
    depot = problem.add_depot(0)
    routing.set_depot_location(depot, 0, 0)

    # Add 3 clients
    for i, (x, y, demand) in enumerate([(10, 20, 5), (30, 10, 3), (20, 30, 4)], start=1):
        client = problem.add_client(i)
        routing.set_client_location(client, x, y)
        routing.set_client_demand(client, demand)

    # Add 1 vehicle with capacity 15
    vehicle = problem.add_vehicle(0)
    routing.set_vehicle_capacity(vehicle, 15)

    # Solve
    solution = routing.solve(problem, "cp", timeout=10)

    # Print result
    if solution:
        print(f"Cost: {solution.cost:.2f}")
        for i, tour in enumerate(solution.get_tours()):
            print(f"Route {i}: {tour.get_client_ids()}")
    else:
        print("No solution found")


if __name__ == "__main__":
    main()
