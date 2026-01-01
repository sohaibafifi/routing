# Python API Reference

The Python bindings provide a high-level interface to the Routing library.

## Installation

```bash
cd python
pip install -e .
```

## Quick Start

```python
import routing

# Initialize the library (loads all plugins)
routing.init()

# List available solvers
print(routing.list_solvers())
# ['ga', 'ma', 'vns', 'pso', 'ls', 'alns', 'mip/cplex', 'mip/highs', 'cp/cplex', 'cp/ortools', ...]
```

---

## Core Functions

### `routing.init()`

Initialize the routing library. Must be called before using any other functionality.

```python
import routing
routing.init()
```

:::{note}
This function registers all plugins and initializes the solver registry.
It only needs to be called once at the start of your program.
:::

---

### `routing.list_solvers()`

Returns a list of available solver names.

```python
solvers = routing.list_solvers()
# ['ga', 'ma', 'vns', 'pso', 'ls', 'alns', 'cp/cplex', 'cp/ortools', 'mip/cplex', 'mip/highs', ...]
```

**Returns:** `List[str]` - List of solver identifiers

---

### `routing.create_solver(name, problem)`

Create a solver instance for a problem.

```python
solver = routing.create_solver("ga", problem)
```

**Parameters:**

| Name | Type | Description |
|------|------|-------------|
| `name` | `str` | Solver name (from `list_solvers()`) |
| `problem` | `Problem` | The problem instance to solve |

**Returns:** `Solver` - A solver instance

**Example:**

```python
problem = routing.load_solomon("c101.txt")
solver = routing.create_solver("ga", problem)
solver.set_param_int("iterMax", 10000)
solver.solve(60.0)
```

---

## Problem Loading

### `routing.load_solomon(filepath)`

Load a Solomon-format CVRPTW instance.

```python
problem = routing.load_solomon("data/CVRPTW/Solomon/10/c101.txt")
```

**Parameters:**

| Name | Type | Description |
|------|------|-------------|
| `filepath` | `str` | Path to Solomon format file (.txt) |

**Returns:** `Problem` - A problem instance with CVRPTW attributes

**Example:**

```python
problem = routing.load_solomon("c101.txt")
print(f"Clients: {problem.num_clients}")
print(f"Vehicles: {problem.num_vehicles}")
```

---

### `routing.load_tsplib(filepath)`

Load a TSPLIB/CVRPLIB format instance.

```python
problem = routing.load_tsplib("data/CVRP/A/A-n32-k5.vrp")
```

**Parameters:**

| Name | Type | Description |
|------|------|-------------|
| `filepath` | `str` | Path to TSPLIB format file (.vrp, .tsp) |

**Returns:** `Problem` - A problem instance

---

## Problem Class

### `routing.Problem`

The main class for defining vehicle routing problems.

```python
problem = routing.Problem()
```

#### Methods

##### `add_depot(id)`

Add a depot to the problem.

```python
depot = problem.add_depot(0)
```

##### `add_client(id)`

Add a client to the problem.

```python
client = problem.add_client(1)
```

##### `add_vehicle(id)`

Add a vehicle to the problem.

```python
vehicle = problem.add_vehicle(0)
```

##### `get_distance(i, j)`

Get the distance between two nodes.

```python
dist = problem.get_distance(0, 1)  # Depot to client 1
```

##### `get_clients()`

Get all clients.

```python
clients = problem.get_clients()
for c in clients:
    print(f"Client {c.get_id()}: ({c.x}, {c.y})")
```

##### `get_vehicles()`

Get all vehicles.

```python
vehicles = problem.get_vehicles()
```

##### `get_depots()`

Get all depots.

```python
depots = problem.get_depots()
```

#### Properties

| Property | Type | Description |
|----------|------|-------------|
| `num_clients` | `int` | Number of clients |
| `num_vehicles` | `int` | Number of vehicles |

---

## Entity Classes

### Client

Represents a customer location with attributes.

| Property | Type | Description |
|----------|------|-------------|
| `x` | `float` | X coordinate |
| `y` | `float` | Y coordinate |
| `demand` | `int` | Demand at this client |
| `tw_open` | `float` | Time window opening |
| `tw_close` | `float` | Time window closing |
| `service_time` | `float` | Service duration |

```python
for client in problem.get_clients():
    print(f"Client at ({client.x}, {client.y})")
    print(f"  Demand: {client.demand}")
    print(f"  TW: [{client.tw_open}, {client.tw_close}]")
```

### Depot

Represents a depot location.

| Property | Type | Description |
|----------|------|-------------|
| `x` | `float` | X coordinate |
| `y` | `float` | Y coordinate |
| `tw_open` | `float` | Operating hours start |
| `tw_close` | `float` | Operating hours end |

### Vehicle

Represents a vehicle with capacity.

| Property | Type | Description |
|----------|------|-------------|
| `capacity` | `int` | Vehicle capacity |

---

## Solver Class

### Methods

#### `solve(timeout)`

Solve the problem with a time limit.

```python
found = solver.solve(60.0)  # 60 second timeout
if found:
    print(f"Solution cost: {solver.get_objective_value()}")
```

**Parameters:**

| Name | Type | Description |
|------|------|-------------|
| `timeout` | `float` | Maximum solving time in seconds |

**Returns:** `bool` - True if a solution was found

#### `get_objective_value()`

Get the objective value of the best solution.

```python
cost = solver.get_objective_value()
```

**Returns:** `float` - Objective value (typically total distance)

#### `get_solution()`

Get the solution object.

```python
solution = solver.get_solution()
```

#### `is_optimal()`

Check if the solution is proven optimal.

```python
if solver.is_optimal():
    print("Optimal solution found!")
```

#### `get_stats()`

Get solver statistics as a string.

```python
print(solver.get_stats())
```

### Parameter Setting

#### `set_param_int(name, value)`

Set an integer parameter.

```python
solver.set_param_int("iterMax", 10000)
```

#### `set_param_bool(name, value)`

Set a boolean parameter.

```python
solver.set_param_bool("feasibleOnly", True)
```

#### `set_param_float(name, value)`

Set a float parameter.

```python
solver.set_param_float("infeasiblePenalty", 1000.0)
```

### Common Parameters

| Parameter | Type | Solvers | Description |
|-----------|------|---------|-------------|
| `iterMax` | int | GA, MA, VNS, PSO, ALNS | Maximum iterations |
| `feasibleOnly` | bool | GA, MA | Only generate feasible solutions |
| `infeasiblePenalty` | float | GA, MA | Penalty for constraint violations |
| `unservedPenalty` | float | GA, MA | Penalty per unserved client |
| `reactionFactor` | float | ALNS | Weight update reaction factor (0-1) |
| `decayFactor` | float | ALNS | Weight decay factor per segment (0-1) |
| `temperature` | float | ALNS | Initial temperature for simulated annealing |
| `coolingRate` | float | ALNS | Temperature cooling rate per iteration |
| `segmentSize` | int | ALNS | Iterations between weight updates |
| `minTemperature` | float | ALNS | Minimum temperature for acceptance |


---

## Solution Class

### Properties

| Property | Type | Description |
|----------|------|-------------|
| `cost` | `float` | Total solution cost |
| `num_routes` | `int` | Number of routes |

### Methods

#### `get_routes()`

Get all routes in the solution.

```python
for route in solution.get_routes():
    print(f"Route cost: {route.cost}")
    for client_id in route.clients:
        print(f"  -> {client_id}")
```

---

## ProblemBuilder

A fluent API for building problems programmatically.

```python
from routing.problem import ProblemBuilder

problem = (ProblemBuilder()
    .with_depot(0, 0, tw_open=0, tw_close=1000)
    .add_client(1, x=10, y=20, demand=5, tw_open=0, tw_close=200, service_time=10)
    .add_client(2, x=15, y=25, demand=3, tw_open=50, tw_close=300, service_time=10)
    .add_client(3, x=20, y=10, demand=8, tw_open=100, tw_close=400, service_time=10)
    .add_vehicles(count=2, capacity=20)
    .build())
```

### Methods

#### `with_depot(x, y, tw_open=0, tw_close=inf)`

Set the depot location and operating hours.

#### `add_client(id, x, y, demand, tw_open=0, tw_close=inf, service_time=0)`

Add a client with all attributes.

#### `add_vehicles(count, capacity)`

Add multiple vehicles with the same capacity.

#### `build()`

Build and return the `Problem` instance.

---

## Complete Example

```python
import routing

# Initialize
routing.init()

# Load instance
problem = routing.load_solomon("data/CVRPTW/Solomon/10/c101.txt")

print(f"Instance: {problem.num_clients} clients, {problem.num_vehicles} vehicles")

# Try different solvers
results = {}

for solver_name in ["ga", "alns", "mip/highs", "cp/ortools"]:
    try:
        solver = routing.create_solver(solver_name, problem)

        if "ga" in solver_name:
            solver.set_param_int("iterMax", 5000)

        if solver.solve(30.0):
            results[solver_name] = solver.get_objective_value()
            print(f"{solver_name}: {results[solver_name]:.2f}")
        else:
            print(f"{solver_name}: No solution")

    except Exception as e:
        print(f"{solver_name}: Error - {e}")

# Best solver
if results:
    best = min(results.items(), key=lambda x: x[1])
    print(f"\nBest: {best[0]} with cost {best[1]:.2f}")
```

---

## Error Handling

```python
import routing

try:
    routing.init()

    # File not found
    problem = routing.load_solomon("nonexistent.txt")

except FileNotFoundError as e:
    print(f"File error: {e}")

except RuntimeError as e:
    print(f"Runtime error: {e}")

except Exception as e:
    print(f"Unexpected error: {e}")
```
