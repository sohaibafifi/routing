# Python API Reference

Modern, type-safe Python bindings for the Routing library with automatic attribute enabling and comprehensive solver support.

---

## 🚀 Quick Start

```python
import routing
from routing.constants import Attribute

# Initialize library
routing.init()

# Create problem
problem = routing.Problem()

# Add depot
depot = problem.add_depot(0)
depot.add_attribute(Attribute.GEONODE, 0, 0)

# Add clients
client = problem.add_client(1)
client.add_attribute(Attribute.GEONODE, 10, 20)
client.add_attribute(Attribute.CONSUMER, 15)

# Add vehicle
vehicle = problem.add_vehicle(0)
vehicle.add_attribute(Attribute.STOCK, 100)

# Solve (attributes auto-enabled!)
solution = routing.solve(problem, "ga", timeout=30)

print(f"Cost: {solution.cost}, Feasible: {solution.is_feasible}")
```

---

## 📦 Installation

```bash
cd python
pip install -e .
```

---

## 🎯 Core Concepts

### Attributes (Type-Safe)

Use enums for auto-completion and type safety:

```python
from routing.constants import Attribute

# Recommended: Using enums
client.add_attribute(Attribute.GEONODE, 10, 20)
client.add_attribute(Attribute.CONSUMER, 5)

# Also works: Using strings (backward compatible)
client.add_attribute("GeoNode", 10, 20)
client.add_attribute("Consumer", 5)
```

**Available Attributes:**

| Enum | String | Parameters | Use Case |
|------|--------|------------|----------|
| `Attribute.GEONODE` | `"GeoNode"` | `x, y` | Coordinates |
| `Attribute.CONSUMER` | `"Consumer"` | `demand` | Client demand |
| `Attribute.STOCK` | `"Stock"` | `capacity` | Vehicle capacity |
| `Attribute.RENDEZVOUS` | `"Rendezvous"` | `open, close` | Time windows |
| `Attribute.SERVICE_QUERY` | `"ServiceQuery"` | `service_time` | Service duration |
| `Attribute.PROFITER` | `"Profiter"` | `profit` | Profit value (TOP) |
| `Attribute.PICKUP` | `"Pickup"` | `demand` | Pickup demand |
| `Attribute.DELIVERY` | `"Delivery"` | `demand` | Delivery demand |
| `Attribute.SOFT_TIME_WINDOWS` | `"SoftTimeWindows"` | `wait, delay` | Soft TW penalties |
| `Attribute.SYNCED` | `"Synced"` | - | Temporal sync |

### Automatic Enabling

Attributes are **automatically enabled** when added to entities - no manual `enable_attributes()` needed!

```python
# Attributes auto-enable on first use
depot.add_attribute(Attribute.GEONODE, 0, 0)  # ✅ GeoNode enabled
client.add_attribute(Attribute.CONSUMER, 15)   # ✅ Consumer enabled

# Just solve - no enable_attributes() call needed!
solution = routing.solve(problem, "ga", timeout=30)
```

---

## 📚 API Reference

### Initialization

#### `routing.init()`

Initialize the routing library. **Must be called before any other operations.**

```python
import routing
routing.init()
```

---

### Discovery Functions

#### `routing.list_solvers()`

Get list of available solver names.

```python
solvers = routing.list_solvers()
# Returns: ['ga', 'ma', 'vns', 'pso', 'ls', 'alns', 'mip', 'cp', ...]
```

#### `routing.list_attributes()`

Get list of available attribute types.

```python
attrs = routing.list_attributes()
# Returns: ['GeoNode', 'Consumer', 'Stock', 'Rendezvous', ...]
```

#### `routing.get_attribute_info()`

Get detailed attribute information.

```python
info = routing.get_attribute_info()
print(info['GeoNode']['description'])
# "Coordinates (x, y) for distance calculation"
print(info['GeoNode']['parameters'])
# ['x: float', 'y: float']
```

#### `routing.get_problem_types()`

Get attribute requirements for common problem types.

```python
types = routing.get_problem_types()
print(types['CVRPTW'])
# ['GeoNode', 'Consumer', 'Stock', 'Rendezvous', 'ServiceQuery']
```

#### `routing.get_solver_info(solver_name)`

Get detailed solver information.

```python
info = routing.get_solver_info('ga')
print(info['description'])
# "Metaheuristic optimization using evolutionary principles"
print(info['parameters'])
# {'iterMax': {...}, 'feasibleOnly': {...}, ...}
```

#### `routing.print_available_resources()`

Print formatted summary of all available resources.

```python
routing.print_available_resources()
# Prints: Solvers, Attributes, Problem Types
```

---

### Problem Class

#### `routing.Problem()`

Create a new routing problem instance.

```python
problem = routing.Problem()
```

**Properties:**

| Property | Type | Description |
|----------|------|-------------|
| `num_clients` | `int` | Number of clients |
| `num_vehicles` | `int` | Number of vehicles |
| `num_depots` | `int` | Number of depots |
| `total_demand` | `int` | Sum of all client demands |
| `total_capacity` | `int` | Sum of all vehicle capacities |

**Methods:**

##### `add_depot(id)`

Add a depot to the problem.

```python
depot = problem.add_depot(0)
depot.add_attribute(Attribute.GEONODE, 0, 0)
```

##### `add_client(id)`

Add a client to the problem.

```python
client = problem.add_client(1)
client.add_attribute(Attribute.GEONODE, 10, 20)
client.add_attribute(Attribute.CONSUMER, 5)
```

##### `add_vehicle(id)`

Add a vehicle to the problem.

```python
vehicle = problem.add_vehicle(0)
vehicle.add_attribute(Attribute.STOCK, 100)
```

##### `get_clients()`, `get_vehicles()`, `get_depots()`

Get lists of all entities.

```python
for client in problem.get_clients():
    print(f"Client {client.get_id()}")

for vehicle in problem.get_vehicles():
    print(f"Vehicle {vehicle.get_id()}")
```

##### `get_depot(id)`

Get depot by ID.

```python
depot = problem.get_depot(0)
```

##### `get_distance(i, j)`

Get distance between two nodes.

```python
dist = problem.get_distance(0, 1)  # Depot to client 1
```

---

### Entity Classes

All entities (Client, Depot, Vehicle) support generic attribute addition.

#### Common Methods

##### `add_attribute(name, *args)`

Add an attribute to the entity. Accepts enum or string.

```python
# Using enum (recommended)
client.add_attribute(Attribute.GEONODE, 10, 20)
client.add_attribute(Attribute.CONSUMER, 15)

# Using string (backward compatible)
client.add_attribute("GeoNode", 10, 20)
client.add_attribute("Consumer", 15)
```

##### `has_attribute(name)`

Check if entity has a specific attribute.

```python
if client.has_attribute(Attribute.GEONODE):
    print(f"Location: ({client.x}, {client.y})")
```

##### `get_id()`

Get entity ID.

```python
id = client.get_id()
```

#### Entity Properties

Properties return `None` if attribute not set.

**Client:**

| Property | Type | Attribute | Description |
|----------|------|-----------|-------------|
| `x` | `float` | GeoNode | X coordinate |
| `y` | `float` | GeoNode | Y coordinate |
| `demand` | `int` | Consumer | Demand |
| `tw_open` | `float` | Rendezvous | Time window open |
| `tw_close` | `float` | Rendezvous | Time window close |
| `service_time` | `float` | ServiceQuery | Service duration |
| `pickup` | `int` | Pickup | Pickup demand |
| `delivery` | `int` | Delivery | Delivery demand |
| `profit` | `float` | Profiter | Profit value |

**Depot:**

| Property | Type | Attribute | Description |
|----------|------|-----------|-------------|
| `x` | `float` | GeoNode | X coordinate |
| `y` | `float` | GeoNode | Y coordinate |
| `tw_open` | `float` | Rendezvous | Operating hours start |
| `tw_close` | `float` | Rendezvous | Operating hours end |

**Vehicle:**

| Property | Type | Attribute | Description |
|----------|------|-----------|-------------|
| `capacity` | `int` | Stock | Vehicle capacity |

---

### Solving

#### `routing.solve(problem, solver_type, timeout, verbose=False)`

Solve a problem with a specific solver.

```python
solution = routing.solve(
    problem,
    solver_type="ga",
    timeout=30.0,
    verbose=True
)
```

**Parameters:**

| Parameter | Type | Description |
|-----------|------|-------------|
| `problem` | `Problem` | Problem instance |
| `solver_type` | `str` | Solver name (from `list_solvers()`) |
| `timeout` | `float` | Max time in seconds |
| `verbose` | `bool` | Print solving progress |

**Returns:** `Solution` object or `None`

#### `routing.solve_with_callback(problem, callback, solver_type, timeout)`

Solve with improvement callback.

```python
improvements = []

def on_improvement(solution, cost):
    improvements.append(cost)
    print(f"New best: {cost:.2f}")

solution = routing.solve_with_callback(
    problem,
    callback=on_improvement,
    solver_type="ga",
    timeout=30.0
)
```

#### `routing.create_solver(solver_type, problem)`

Create solver instance for manual control.

```python
solver = routing.create_solver("ga", problem)
solver.set_param_int("iterMax", 10000)
solver.set_param_bool("feasibleOnly", True)
solver.solve(60.0)

solution = solver.get_solution()
is_optimal = solver.is_optimal()
stats = solver.get_stats()
```

---

### Solution Class

#### Properties

| Property | Type | Description |
|----------|------|-------------|
| `cost` | `float` | Total solution cost |
| `total_distance` | `float` | Alias for cost |
| `num_tours` | `int` | Number of routes |
| `is_feasible` | `bool` | All clients served |
| `unserved` | `list[int]` | List of unserved client IDs |

#### Methods

##### `get_tours()`

Get all routes.

```python
for tour in solution.get_tours():
    print(f"Tour cost: {tour.cost}")
    print(f"Clients: {tour.get_client_ids()}")
```

##### `get_tour(index)`

Get route by index.

```python
tour = solution.get_tour(0)
```

##### `to_dict()`

Convert to dictionary.

```python
data = solution.to_dict()
# {
#   'cost': 123.45,
#   'num_tours': 3,
#   'is_feasible': True,
#   'tours': [...],
#   'unserved': []
# }
```

##### `clone()`

Create a deep copy.

```python
solution_copy = solution.clone()
```

---

### Tour Class

#### Properties

| Property | Type | Description |
|----------|------|-------------|
| `cost` | `float` | Route cost |
| `total_distance` | `float` | Alias for cost |
| `total_demand` | `int` | Sum of client demands |

#### Methods

##### `get_client_ids()`

Get list of client IDs in route.

```python
clients = tour.get_client_ids()
# [1, 3, 5, 7]
```

##### `to_list()`

Alias for `get_client_ids()`.

```python
clients = tour.to_list()
```

---

### Solver Parameters

#### Setting Parameters

```python
solver.set_param_int("iterMax", 10000)
solver.set_param_bool("feasibleOnly", True)
solver.set_param_double("infeasiblePenalty", 1000.0)
```

#### Common Parameters

| Parameter | Type | Solvers | Description |
|-----------|------|---------|-------------|
| `iterMax` | int | GA, MA, VNS, PSO, ALNS | Maximum iterations |
| `feasibleOnly` | bool | GA, MA | Only feasible solutions |
| `infeasiblePenalty` | float | GA, MA | Constraint violation penalty |
| `unservedPenalty` | float | GA, MA | Unserved client penalty |
| `reactionFactor` | float | ALNS | Weight update reaction (0-1) |
| `decayFactor` | float | ALNS | Weight decay per segment (0-1) |
| `temperature` | float | ALNS | Initial SA temperature |
| `coolingRate` | float | ALNS | Temperature cooling rate |
| `segmentSize` | int | ALNS | Iterations between updates |
| `minTemperature` | float | ALNS | Minimum temperature |

---

### File Loaders

#### `routing.load_solomon(filepath)`

Load Solomon CVRPTW instance.

```python
problem = routing.load_solomon("data/c101.txt")
print(f"{problem.num_clients} clients")
```

#### `routing.load_tsplib(filepath)`

Load TSPLIB/CVRPLIB instance.

```python
problem = routing.load_tsplib("data/A-n32-k5.vrp")
```

---

### ProblemBuilder (High-Level API)

Fluent interface for building problems.

```python
from routing import ProblemBuilder

problem = (ProblemBuilder()
    .with_depot(0, 0, tw_open=0, tw_close=1000)
    .add_client(x=10, y=20, demand=5, tw_open=0, tw_close=200, service_time=10)
    .add_client(x=30, y=40, demand=3, tw_open=50, tw_close=300, service_time=10)
    .add_vehicles(count=2, capacity=50)
    .build())

solution = routing.solve(problem, "ga", timeout=30)
```

**Methods:**

- `with_depot(x, y, tw_open=None, tw_close=None)`
- `add_client(x, y, demand, tw_open=None, tw_close=None, service_time=None)`
- `add_vehicles(count, capacity)`
- `build()` - Returns Problem instance

---

## 🎓 Complete Examples

### Simple CVRP

```python
import routing
from routing.constants import Attribute

routing.init()

problem = routing.Problem()

# Depot
depot = problem.add_depot(0)
depot.add_attribute(Attribute.GEONODE, 0, 0)

# Clients
for i in range(1, 11):
    client = problem.add_client(i)
    client.add_attribute(Attribute.GEONODE, i * 10, i * 10)
    client.add_attribute(Attribute.CONSUMER, 5 + i)

# Vehicles
for v in range(3):
    vehicle = problem.add_vehicle(v)
    vehicle.add_attribute(Attribute.STOCK, 50)

# Solve
solution = routing.solve(problem, "ga", timeout=30)

print(f"Cost: {solution.cost:.2f}")
print(f"Feasible: {solution.is_feasible}")
print(f"Routes: {solution.num_tours}")
```

### CVRPTW with Time Windows

```python
from routing import ProblemBuilder

problem = (ProblemBuilder()
    .with_depot(0, 0, tw_open=0, tw_close=1000)
    .add_client(x=20, y=20, demand=10, tw_open=0, tw_close=100, service_time=10)
    .add_client(x=30, y=40, demand=15, tw_open=10, tw_close=80, service_time=10)
    .add_client(x=50, y=30, demand=20, tw_open=20, tw_close=90, service_time=10)
    .add_vehicles(count=2, capacity=50)
    .build())

solution = routing.solve(problem, "alns", timeout=60)

for i, tour in enumerate(solution.get_tours()):
    route = " -> ".join(["0"] + [str(c) for c in tour.get_client_ids()] + ["0"])
    print(f"Vehicle {i}: {route} (cost: {tour.cost:.2f})")
```

### Multi-Solver Comparison

```python
routing.init()
problem = routing.load_solomon("c101.txt")

results = {}
for solver in ["ga", "alns", "vns"]:
    solution = routing.solve(problem, solver, timeout=30)
    if solution:
        results[solver] = solution.cost

# Print results
for solver, cost in sorted(results.items(), key=lambda x: x[1]):
    print(f"{solver:10} - Cost: {cost:.2f}")
```

### Using Solver Directly

```python
solver = routing.create_solver("ga", problem)

# Configure parameters
solver.set_param_int("iterMax", 5000)
solver.set_param_bool("feasibleOnly", True)
solver.set_param_double("infeasiblePenalty", 1000.0)

# Solve
if solver.solve(60.0):
    print(f"Cost: {solver.get_objective_value():.2f}")
    print(f"Optimal: {solver.is_optimal()}")
    print(f"Stats:\n{solver.get_stats()}")

    solution = solver.get_solution()
```

---

## 🔍 Discovery API Examples

```python
import routing

routing.init()

# List all available resources
routing.print_available_resources()

# Check attribute details
info = routing.get_attribute_info()
for attr_name, details in info.items():
    print(f"{attr_name}: {details['description']}")

# Get problem type requirements
types = routing.get_problem_types()
print(f"CVRPTW needs: {types['CVRPTW']}")

# Get solver information
ga_info = routing.get_solver_info('ga')
print(f"GA: {ga_info['description']}")
print(f"Parameters: {list(ga_info['parameters'].keys())}")
```

---

## 🎨 Best Practices

1. **Use enums for attributes** - Better IDE support and type safety
   ```python
   from routing.constants import Attribute
   client.add_attribute(Attribute.GEONODE, 10, 20)  # ✅
   client.add_attribute("GeoNode", 10, 20)          # ⚠️ Works but less safe
   ```

2. **Always call `routing.init()`** before using the library

3. **Use ProblemBuilder** for simple problems - cleaner syntax

4. **Leverage discovery functions** - `list_solvers()`, `list_attributes()`

5. **Check solution feasibility** - `solution.is_feasible` before using results

6. **Use `solve_with_callback()`** for long runs - monitor progress

---

## 📖 See Also

- [Attributes Guide](attributes.md) - Detailed attribute documentation
- [Solver Guide](solvers.md) - Solver comparison and tuning
- [C++ API](cpp-api.md) - C++ interface documentation

---

**Version:** 0.1.1
**Last Updated:** 2026-01-18
