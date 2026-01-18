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
| CPLEX | Commercial MIP/CP solver | `mip/cplex`, `cp/cpo` |
| HiGHS | Open-source MIP solver | `mip/highs` |
| OR-Tools | Open-source CP-SAT solver | `cp/ortools` |
| [XCSP3/ACE](https://www.xcsp.org/) | CP modeling format export | `cp/ace` |

---

## Installation

::::{tab-set}

:::{tab-item} Python (Recommended)
:sync: python

### Quick Install

```bash
# Clone the repository
git clone https://github.com/sohaibafifi/rihla.git
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
Available solvers: ['ga', 'ma', 'vns', 'cp/cpo', 'cp/ortools', 'mip/cplex', 'mip/highs', ...]
```
:::

:::{tab-item} C++ (CMake)
:sync: cpp

### Build from Source

```bash
# Clone with submodules
git clone https://github.com/sohaibafifi/rihla.git
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
from routing.constants import Attribute

# Initialize the library
routing.init()

# Create problem
problem = routing.Problem()

# Add depot with location and time window
depot = problem.add_depot(0)
depot.add_attribute(Attribute.GEONODE, x=0, y=0)
depot.add_attribute(Attribute.RENDEZVOUS, open=0, close=1000)

# Add clients with location, demand, time window, and service time
for i, (x, y) in enumerate([(10, 0), (0, 10), (10, 10)], start=1):
    client = problem.add_client(i)
    client.add_attribute(Attribute.GEONODE, x=x, y=y)
    client.add_attribute(Attribute.CONSUMER, demand=5)
    client.add_attribute(Attribute.RENDEZVOUS, open=0, close=200)
    client.add_attribute(Attribute.SERVICE_QUERY, service_time=10)

# Add vehicles with capacity
for v in range(2):
    vehicle = problem.add_vehicle(v)
    vehicle.add_attribute(Attribute.STOCK, capacity=20)

# Solve with genetic algorithm (attributes are auto-enabled!)
solution = routing.solve(problem, "ga", timeout=10)

if solution:
    print(f"Solution found!")
    print(f"Total distance: {solution.cost:.2f}")
    for i, tour in enumerate(solution.get_tours()):
        print(f"  Route {i}: {tour.get_client_ids()}")
else:
    print("No solution found")
```
:::

:::{tab-item} C++
:sync: cpp

```cpp
#include <routing/routing.hpp>

int main() {
    routing::Problem problem;

    // Add depot with location and time window
    auto* depot = problem.addDepot(0);
    depot->addAttribute<routing::attributes::GeoNode>(0.0, 0.0);
    depot->addAttribute<routing::attributes::Rendezvous>(0.0, 1000.0);

    // Add clients
    double coords[][2] = {{10.0, 0.0}, {0.0, 10.0}, {10.0, 10.0}};
    for (int i = 1; i <= 3; ++i) {
        auto* client = problem.addClient(i);
        client->addAttribute<routing::attributes::GeoNode>(coords[i-1][0], coords[i-1][1]);
        client->addAttribute<routing::attributes::Consumer>(5);
        client->addAttribute<routing::attributes::Rendezvous>(0.0, 200.0);
        client->addAttribute<routing::attributes::ServiceQuery>(10.0);
    }

    // Add vehicles
    for (int k = 0; k < 2; ++k) {
        auto* vehicle = problem.addVehicle(k);
        vehicle->addAttribute<routing::attributes::Stock>(20);
    }

    // Solve (attributes are auto-enabled!)
    routing::GASolver solver(&problem);
    if (solver.solve(10.0)) {
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

| Category | Solvers | Best For |
|----------|---------|----------|
| **Metaheuristics** | `ga`, `ma`, `vns`, `alns` | Large instances, fast results |
| **MIP** | `mip/cplex`, `mip/highs` | Small instances, proven optimality |
| **CP** | `cp/cpo`, `cp/ortools` | Complex constraints |

```python
# Metaheuristic (default choice)
solution = routing.solve(problem, "ga", timeout=60)

# Open-source MIP solver
solution = routing.solve(problem, "mip/highs", timeout=300)
```

---

## Next Steps

- **[Attributes](attributes.md)**: Available problem attributes
- **[Solvers](solvers.md)**: Detailed solver configuration
- **[Python API](python-api.md)**: Complete Python reference
- **[C++ API](cpp-api.md)**: Complete C++ reference

---

## Common Issues

### "No solution found"

1. Check time limit - metaheuristics need time
2. Verify problem is feasible (capacity, time windows)

### Import Error

```
ImportError: routing._routing_core not found
```

Rebuild the C++ extension:

```bash
cd python
pip install -e . --no-build-isolation
```
