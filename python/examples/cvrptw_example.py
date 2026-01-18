#!/usr/bin/env python3
"""
CVRPTW Example - Capacitated Vehicle Routing Problem with Time Windows

This example demonstrates how to:
1. Create a CVRPTW problem from scratch
2. Add clients with location, demand, time windows, and service time
3. Add vehicles with capacity
4. Solve using different solvers
5. Extract and display the solution

Problem: Deliver goods to 10 customers with time windows using vehicles
with limited capacity, minimizing total travel distance.
"""

import routing
from routing.problem import ProblemBuilder


def create_simple_cvrptw():
    """Create a simple CVRPTW instance with 10 customers."""

    # Customer data: (id, x, y, demand, tw_open, tw_close, service_time)
    customers = [
        (1, 20, 20, 10, 0, 100, 10),
        (2, 30, 40, 15, 10, 80, 10),
        (3, 50, 30, 20, 20, 90, 10),
        (4, 40, 60, 10, 30, 100, 10),
        (5, 60, 50, 25, 0, 90, 10),
        (6, 70, 20, 15, 40, 120, 10),
        (7, 35, 35, 10, 0, 100, 10),
        (8, 25, 55, 20, 50, 110, 10),
        (9, 55, 45, 15, 20, 80, 10),
        (10, 45, 25, 10, 0, 100, 10),
    ]

    # Create problem using builder
    builder = ProblemBuilder()

    # Add depot at origin with time window
    builder.with_depot(0, 0, tw_open=0, tw_close=200)

    # Add all customers
    for cid, x, y, demand, tw_open, tw_close, service in customers:
        builder.add_client(
            client_id=cid,
            x=x,
            y=y,
            demand=demand,
            tw_open=tw_open,
            tw_close=tw_close,
            service_time=service
        )

    # Add 3 vehicles with capacity 50
    builder.add_vehicles(3, capacity=50)

    problem = builder.build()
    # Attributes are automatically enabled when added
    return problem


def create_cvrptw_manual():
    """Create the same problem using the generic attribute API."""

    problem = routing.Problem()

    # Add depot with location and time window
    depot = problem.add_depot(0)
    depot.add_attribute("GeoNode", 0, 0)
    depot.add_attribute("Rendezvous", 0, 200)

    # Customer data
    customers = [
        (1, 20, 20, 10, 0, 100, 10),
        (2, 30, 40, 15, 10, 80, 10),
        (3, 50, 30, 20, 20, 90, 10),
        (4, 40, 60, 10, 30, 100, 10),
        (5, 60, 50, 25, 0, 90, 10),
        (6, 70, 20, 15, 40, 120, 10),
        (7, 35, 35, 10, 0, 100, 10),
        (8, 25, 55, 20, 50, 110, 10),
        (9, 55, 45, 15, 20, 80, 10),
        (10, 45, 25, 10, 0, 100, 10),
    ]

    # Add customers with attributes
    for cid, x, y, demand, tw_open, tw_close, service in customers:
        client = problem.add_client(cid)
        client.add_attribute("GeoNode", x, y)
        client.add_attribute("Consumer", demand)
        client.add_attribute("Rendezvous", tw_open, tw_close)
        client.add_attribute("ServiceQuery", service)

    # Add vehicles with capacity
    for vid in range(3):
        vehicle = problem.add_vehicle(vid)
        vehicle.add_attribute("Stock", 50)

    # Attributes are automatically enabled when added
    return problem


def print_solution(solution):
    """Pretty print a solution."""
    if solution is None:
        print("No solution found!")
        return

    print(f"\n{'='*50}")
    print(f"Solution Summary")
    print(f"{'='*50}")
    print(f"Total Cost: {solution.cost:.2f}")
    print(f"Number of Routes: {solution.num_tours}")
    print(f"Unserved Customers: {len(solution.unserved)}")

    if solution.unserved:
        print(f"  Unserved IDs: {solution.unserved}")

    print(f"\n{'='*50}")
    print("Routes:")
    print(f"{'='*50}")

    for i, tour in enumerate(solution.get_tours()):
        client_ids = tour.get_client_ids()
        if client_ids:
            route_str = " -> ".join(["0"] + [str(c) for c in client_ids] + ["0"])
            print(f"  Vehicle {i}: {route_str}")
            print(f"            Cost: {tour.cost:.2f}, Clients: {len(client_ids)}")

    print(f"{'='*50}\n")


def main():
    """Main example function."""
    print("CVRPTW Example - Routing Library")
    print("=" * 50)

    # Initialize the routing library
    routing.init()

    # Create problem
    print("\n1. Creating CVRPTW problem...")
    problem = create_simple_cvrptw()
    print(f"   Created problem with {problem.num_clients} clients and {problem.num_vehicles} vehicles")

    # List available solvers
    print("\n2. Available solvers:")
    for solver_name in routing.list_solvers():
        print(f"   - {solver_name}")



    results = []

    # Solve with MIP
    print("3. Solving with mip")
    solution_mip = routing.solve(problem, "mip/highs", timeout=30, verbose=True)
    print_solution(solution_mip)
    results.append(("MIP", solution_mip.cost if solution_mip else float('inf')))

    # Solve with Genetic Algorithm
    print("4. Solving with ga")
    solution_ga = routing.solve(problem, "ga", timeout=30, verbose=True)
    print_solution(solution_ga)
    results.append(("Genetic Algorithm", solution_ga.cost if solution_ga else float('inf')))

    # Solve using CP Solver
    print("5. Solving with cp")
    solution_cp = routing.solve(problem, "cp/cpo", timeout=30, verbose=True)
    print_solution(solution_cp)
    results.append(("Constraint Programming", solution_cp.cost if solution_cp else float('inf')))

    # Solve using xcsp Solver
    print("6. Solving with xcsp3")
    solution_xcsp = routing.solve(problem, "cp/ace", timeout=30, verbose=True)
    print_solution(solution_xcsp)
    results.append(("XCSP3", solution_xcsp.cost if solution_xcsp else float('inf')))

    # Print summary table
    print("\n" + "="*40)
    print(f"{'Solver':<25} | {'Cost':>10}")
    print("-" * 40)
    for solver, cost in results:
        cost_str = f"{cost:.2f}" if cost != float('inf') else "No Solution"
        print(f"{solver:<25} | {cost_str:>10}")
    print("="*40 + "\n")




def example_with_solver_object():
    """Example using the Solver class for more control."""
    from routing.solver import Solver

    problem = create_simple_cvrptw()

    # Create solver with configuration
    solver = Solver("ga", problem)
    solver.timeout = 60

    # Solve
    solution = solver.solve()

    if solution:
        print(f"Cost: {solution.cost}")
        print(f"Optimal: {solver.is_optimal}")
        print(f"Stats:\n{solver.stats}")


if __name__ == "__main__":
    main()
