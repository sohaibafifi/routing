# Routing Library - Python Bindings

A composable, multi-solver VRP framework with Python bindings.

## Installation

### From Source

```bash
cd python
pip install .
```

### Development Install

```bash
pip install -e ".[dev]"
```

## Quick Start

```python
import routing
from routing.constants import Attribute

routing.init()

# Create a problem
problem = routing.Problem()

# Add depot
depot = problem.add_depot(0)
depot.add_attribute(Attribute.GEONODE, 0, 0)

# Add clients
c1 = problem.add_client(1)
c1.add_attribute(Attribute.GEONODE, 10, 20)
c1.add_attribute(Attribute.CONSUMER, 5)

c2 = problem.add_client(2)
c2.add_attribute(Attribute.GEONODE, 30, 40)
c2.add_attribute(Attribute.CONSUMER, 3)

# Add vehicle
v = problem.add_vehicle(0)
v.add_attribute(Attribute.STOCK, 100)

# Solve
solution = routing.solve(problem, "ga", timeout=30)

# Results
if solution:
    print(f"Cost: {solution.cost}")
    for tour in solution.get_tours():
        print(f"Tour: {tour.get_client_ids()}")
```

## Using the Builder API

```python
from routing.problem import ProblemBuilder

problem = (ProblemBuilder()
    .with_depot(0, 0)
    .add_client(1, x=10, y=20, demand=5)
    .add_client(2, x=30, y=40, demand=3, tw_open=8, tw_close=12)
    .add_vehicles(3, capacity=100)
    .build())

solution = routing.solve(problem, "vns", timeout=60)
```

## Available Solvers

- `ga` - Genetic Algorithm
- `vns` - Variable Neighborhood Search
- `ma` - Memetic Algorithm
- `pso` - Particle Swarm Optimization
- `ls` - Local Search
- `mip` - Mixed Integer Programming (CPLEX)
- `cp` - Constraint Programming (CP Optimizer)

## Available Attributes

- `GeoNode` - x, y coordinates
- `Consumer` - demand
- `Stock` - vehicle capacity
- `TimeWindow` - open/close times
- `ServiceTime` - service duration
- `Profit` - profit value (for TOP problems)

## API Reference

### Core Classes

- `Problem` - VRP problem definition
- `Solution` - Solution with tours
- `Tour` - Single vehicle route
- `Client` - Customer node
- `Vehicle` - Vehicle with attributes
- `Depot` - Depot node

### Functions

- `routing.solve(problem, solver_type, timeout)` - Quick solve
- `routing.create_solver(solver_type, problem)` - Create solver instance
- `routing.list_solvers()` - List available solvers

## License

You are allowed to retrieve this project for research purposes as a member of a non-commercial and academic institution.
