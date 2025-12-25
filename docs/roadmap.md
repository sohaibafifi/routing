# Routing Library Roadmap

A composable, multi-solver VRP framework with C++ core and Python bindings.

## Current Status

### Completed

| Feature | Description |
|---------|-------------|
| Composable Problem API | ECS-inspired attribute system for building any VRP variant |
| Multiple Solvers | GA, VNS, MA, PSO, Local Search, MIP (CPLEX) |
| CP Backend | Constraint Programming with IBM CP Optimizer |
| Plugin Architecture | Extensible attributes, constraints, and solvers |
| Solomon/TSPLIB Readers | Standard benchmark instance formats |

### In Progress

| Feature | Description |
|---------|-------------|
| Python Bindings | Native Python API using nanobind |

### Planned

| Feature | Priority | Description |
|---------|----------|-------------|
| Hybrid CP + LNS | High | CP-based repair in Large Neighborhood Search |
| Incremental Evaluation | High | O(1) move cost evaluation with caching |
| ALNS Solver | Medium | Adaptive Large Neighborhood Search |
| HiGHS Backend | Medium | Open-source MIP solver support |
| JSON/YAML Format | Medium | Universal instance format |
| Parallel Solving | Medium | Multi-threaded solver orchestration |

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                    Python API                       │
├─────────────────────────────────────────────────────┤
│                   C++ Core Library                  │
│  ┌─────────────────────────────────────────────┐    │
│  │              Problem Layer                  │    │
│  │   Attributes │ Constraints │ Evaluators     │    │
│  └─────────────────────────────────────────────┘    │
│  ┌─────────────────────────────────────────────┐    │
│  │              Solver Layer                   │    │
│  │   GA │ VNS │ ALNS │ SA │ Hybrid │ CP │ MIP  │    │
│  └─────────────────────────────────────────────┘    │
│  ┌─────────────────────────────────────────────┐    │
│  │           Optimization Backends             │    │
│  │   CPLEX │ GUROBI │ HiGHS │ CP Optimizer     │    │
│  └─────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────┘
```

## Quick Start (Coming Soon)

```python
import routing

# Load problem
problem = routing.Problem.load("instance.json")

# Or compose manually
problem = routing.Problem()
problem.add_attribute("capacity")
problem.add_attribute("time_windows")

# Solve
solver = routing.Solver("genetic", timeout=60)
solution = solver.solve(problem)

# Results
print(f"Cost: {solution.cost}")
for route in solution.routes:
    print(f"Vehicle {route.id}: {route.nodes}")
```

## Available Attributes

| Attribute | Purpose |
|-----------|---------|
| `GeoNode` | x, y coordinates for distance |
| `Consumer` | Demand at node |
| `Stock` | Vehicle capacity |
| `Rendezvous` | Time window bounds |
| `ServiceQuery` | Service time |
| `Profiter` | Profit (TOP problems) |
| `Pickup`, `Delivery` | P&D demands |
| `Synced` | Temporal synchronization |


## License

You are allowed to retrieve this project for research purposes as a member of a non-commercial and academic institution.
