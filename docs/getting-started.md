# Getting Started

This guide walks you through installing and using the Routing library for the first time.

## Prerequisites

### For Python Users

- Python 3.9 or later
- pip

### For C++ Users

- CMake 3.16+
- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- Git

### Optional Dependencies

| Dependency | Purpose | Required For |
|------------|---------|--------------|
| CPLEX | Commercial MIP/CP solver | `mip/cplex`, `cp/cplex` |
| HiGHS | Open-source MIP solver | `mip/highs` |
| OR-Tools | Open-source CP-SAT solver | `cp/ortools` |
| [XCSP3](https://www.xcsp.org/) | CP modeling format export | `cp/xcsp3` |

---

## Installation

::::{tab-set}

:::{tab-item} Python (Recommended)
:sync: python

### Quick Install

```bash
# Clone the repository
git clone https://github.com/sohaibafifi/routing.git
cd routing

# Install Python package
cd python
pip install -e .
```

### Verify Installation

```python
import routing

# Initialize
routing.init()

# Check available solvers
print("Available solvers:", routing.list_solvers())
```

Expected output:

```
Available solvers: ['ga', 'ma', 'vns', 'cp/cplex', 'cp/ortools', 'mip/cplex', 'mip/highs', ...]
```
:::

:::{tab-item} C++ (CMake)
:sync: cpp

### Build from Source

```bash
# Clone with submodules
git clone https://github.com/sohaibafifi/routing.git
cd routing
git submodule update --init --recursive

# Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --parallel
```

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `ROUTING_BUILD_EXAMPLES` | ON | Build example CLIs |
| `BUILD_TESTING` | ON | Build test suite |
| `ROUTING_BUILD_CPOPTIMIZER` | ON | Build CP Optimizer backend |
| `ROUTING_BUILD_GRB` | OFF | Build Gurobi support |

### Verify Installation

```bash
# Run tests
ctest --test-dir build --output-on-failure

# Run example
./build/examples/composable/example_composable
```
:::

::::

---

## Your First Problem

Let's solve a small Capacitated Vehicle Routing Problem with Time Windows (CVRPTW).

::::{tab-set}

:::{tab-item} Python
:sync: python

```python
import routing
from routing.problem import ProblemBuilder

# Initialize the library
routing.init()

# Build a small problem
problem = (ProblemBuilder()
    # Depot at origin, open 0-1000
    .with_depot(0, 0, tw_open=0, tw_close=1000)

    # 3 clients with coordinates, demand, and time windows
    .add_client(1, x=10, y=0, demand=5, tw_open=0, tw_close=200, service_time=10)
    .add_client(2, x=0, y=10, demand=5, tw_open=0, tw_close=200, service_time=10)
    .add_client(3, x=10, y=10, demand=5, tw_open=0, tw_close=200, service_time=10)

    # 2 vehicles with capacity 20
    .add_vehicles(count=2, capacity=20)
    .build())

# Solve with genetic algorithm
solver = routing.create_solver("ga", problem)
solver.set_param_int("iterMax", 1000)

if solver.solve(10.0):
    print(f"Solution found!")
    print(f"Total distance: {solver.get_objective_value():.2f}")

    # Print routes
    solution = solver.get_solution()
    for i, route in enumerate(solution.get_routes()):
        print(f"  Route {i}: {route.clients}")
else:
    print("No solution found")
```
:::

:::{tab-item} C++
:sync: cpp

```cpp
#include <iostream>
#include <plugins/PluginBundle.hpp>
#include <plugins/attributes/ComposableCorePlugin/Problem.hpp>
#include <plugins/solvers/GASolverPlugin/GASolver.hpp>

int main() {
    // Register and initialize plugins
    routing::plugins::registerAllPlugins(routing::PluginRegistry::instance());
    routing::PluginRegistry::instance().initializeAll();

    // Create problem
    routing::Problem problem;

    // Enable CVRPTW attributes
    problem.enableAttributes<
        routing::attributes::GeoNode,
        routing::attributes::Consumer,
        routing::attributes::Stock,
        routing::attributes::Rendezvous,
        routing::attributes::ServiceQuery>();

    // Add depot
    auto* depot = problem.addDepot(0);
    depot->addAttribute<routing::attributes::GeoNode>(0.0, 0.0);
    depot->addAttribute<routing::attributes::Rendezvous>(0.0, 1000.0);

    // Add clients
    for (int i = 1; i <= 3; ++i) {
        auto* client = problem.addClient(i);
        client->addAttribute<routing::attributes::GeoNode>(
            i == 2 ? 0.0 : 10.0,
            i == 1 ? 0.0 : 10.0);
        client->addAttribute<routing::attributes::Consumer>(5);
        client->addAttribute<routing::attributes::Rendezvous>(0.0, 200.0);
        client->addAttribute<routing::attributes::ServiceQuery>(10.0);
    }

    // Add vehicles
    for (int k = 0; k < 2; ++k) {
        auto* vehicle = problem.addVehicle(k);
        vehicle->addAttribute<routing::attributes::Stock>(20);
    }

    // Solve
    routing::GASolver solver(&problem);
    solver.setDefaultConfiguration();

    if (solver.solve(10.0)) {
        std::cout << "Solution found!" << std::endl;
        std::cout << "Total distance: " << solver.getObjectiveValue() << std::endl;
    }

    return 0;
}
```
:::

::::

---

## Loading Benchmark Instances

The library supports common VRP benchmark formats.

### Solomon Format (CVRPTW)

```python
# Load a Solomon instance
problem = routing.load_solomon("data/CVRPTW/Solomon/100/c101.txt")
print(f"Clients: {problem.num_clients}")
```

### TSPLIB Format (CVRP)

```python
# Load a TSPLIB/CVRPLIB instance
problem = routing.load_tsplib("data/CVRP/A/A-n32-k5.vrp")
```

---

## Choosing a Solver

The library offers multiple solvers for different needs:

### Metaheuristics

Best for large instances where optimal solutions aren't required.

| Solver | Name | Best For |
|--------|------|----------|
| Genetic Algorithm | `ga` | General purpose, good balance |
| Memetic Algorithm | `ma` | Higher quality, slower |
| VNS | `vns` | Fast local improvement |

```python
solver = routing.create_solver("ga", problem)
solver.set_param_int("iterMax", 10000)
solver.solve(60.0)
```

### Exact Methods

Best for small instances or when proven optimality is needed.

| Solver | Name | Backend |
|--------|------|---------|
| MIP (CPLEX) | `mip/cplex` | IBM CPLEX |
| MIP (HiGHS) | `mip/highs` | Open-source |
| CP (CPLEX) | `cp/cplex` | IBM CP Optimizer |
| CP (OR-Tools) | `cp/ortools` | Google OR-Tools |
| CP ([XCSP3](https://www.xcsp.org/)) | `cp/xcsp3` | Export to XML format |

```python
# Use HiGHS (open-source)
solver = routing.create_solver("mip/highs", problem)
solver.solve(300.0)  # 5 minute time limit
```

### XCSP3 Export

Export your problem to [XCSP3](https://www.xcsp.org/) format for use with any XCSP3-compatible solver:

```python
solver = routing.create_solver("cp/xcsp3", problem)
solver.export_model("my_problem.xml")
```

---

## Next Steps

- **[Concepts](concepts.md)**: Learn about the composable attribute system
- **[Solvers](solvers.md)**: Detailed solver configuration and parameters
- **[Attributes](attributes.md)**: Available problem attributes
- **[Python API](python-api.md)**: Complete Python reference
- **[C++ API](cpp-api.md)**: Complete C++ reference

---

## Common Issues

### "No solution found"

1. Check time limit - metaheuristics need time
2. Verify problem is feasible (capacity, time windows)
3. Try `feasibleOnly=False` for GA/MA to see intermediate solutions

### Import Error

```
ImportError: routing._routing_core not found
```

The C++ extension wasn't built. Run:

```bash
cd python
pip install -e . --no-build-isolation
```

### Solver Not Available

```python
>>> routing.list_solvers()
['ga', 'ma', 'vns']  # Missing mip/cplex, cp/cplex
```

CPLEX not found during build. Install CPLEX and rebuild:

```bash
cmake -S . -B build -DCPLEX_ROOT=/path/to/cplex
cmake --build build
```
