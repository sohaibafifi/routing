# Python API Reference

Modern Python bindings for the Routing library with automatic attribute enabling.

---

## Quick Start

```python
import routing
from routing.constants import Attribute

routing.init()

# Create problem
problem = routing.Problem()

# Add depot
depot = problem.add_depot(0)
depot.add_attribute(Attribute.GEONODE, 0, 0)

# Add client
client = problem.add_client(1)
client.add_attribute(Attribute.GEONODE, 10, 20)
client.add_attribute(Attribute.CONSUMER, 15)

# Add vehicle
vehicle = problem.add_vehicle(0)
vehicle.add_attribute(Attribute.STOCK, 100)

# Solve (attributes auto-enabled!)
solution = routing.solve(problem, "ga", timeout=30)
print(f"Cost: {solution.cost}")
```

---

## Attributes

Add attributes using the `Attribute` enum from `routing.constants`:

```python
from routing.constants import Attribute

client.add_attribute(Attribute.GEONODE, 10, 20)       # Location
client.add_attribute(Attribute.CONSUMER, 5)           # Demand
client.add_attribute(Attribute.RENDEZVOUS, 0, 100)    # Time window
client.add_attribute(Attribute.SERVICE_QUERY, 10)     # Service time
```

| Enum | Parameters | Description |
|------|------------|-------------|
| `Attribute.GEONODE` | `x, y` | Coordinates |
| `Attribute.CONSUMER` | `demand` | Client demand |
| `Attribute.STOCK` | `capacity` | Vehicle capacity |
| `Attribute.RENDEZVOUS` | `open, close` | Time window |
| `Attribute.SERVICE_QUERY` | `service_time` | Service duration |
| `Attribute.PROFITER` | `profit` | Profit value (TOP) |
| `Attribute.PICKUP` | `demand` | Pickup demand |
| `Attribute.DELIVERY` | `demand` | Delivery demand |

Attributes are **automatically enabled** when added—no manual setup required.

---

## Problem Class

### Creating a Problem

```python
problem = routing.Problem()
```

### Adding Entities

```python
depot = problem.add_depot(0)
client = problem.add_client(1)
vehicle = problem.add_vehicle(0)
```

### Properties

| Property | Description |
|----------|-------------|
| `num_clients` | Number of clients |
| `num_vehicles` | Number of vehicles |
| `num_depots` | Number of depots |

---

## Entity Methods

All entities (Client, Depot, Vehicle) share these methods:

```python
from routing.constants import Attribute

# Add attribute
entity.add_attribute(Attribute.GEONODE, 10, 20)

# Check if attribute exists
if entity.has_attribute(Attribute.GEONODE):
    print(f"Location: ({entity.x}, {entity.y})")

# Get entity ID
id = entity.get_id()
```

### Entity Properties

**Client:**

| Property | Attribute | Description |
|----------|-----------|-------------|
| `x`, `y` | GeoNode | Coordinates |
| `demand` | Consumer | Demand |
| `tw_open`, `tw_close` | Rendezvous | Time window |
| `service_time` | ServiceQuery | Service duration |
| `profit` | Profiter | Profit value |

**Vehicle:**

| Property | Attribute | Description |
|----------|-----------|-------------|
| `capacity` | Stock | Capacity |

---

## Solving

### Simple Solve

```python
solution = routing.solve(problem, "ga", timeout=30)

if solution:
    print(f"Cost: {solution.cost}")
    print(f"Feasible: {solution.is_feasible}")
```

### With Progress Callback

```python
def on_improvement(solution, cost):
    print(f"New best: {cost:.2f}")

solution = routing.solve_with_callback(
    problem, on_improvement, "ga", timeout=30
)
```

### Manual Solver Control

```python
solver = routing.create_solver("ga", problem)
solver.set_param_int("iterMax", 10000)
solver.solve(60.0)

solution = solver.get_solution()
print(f"Optimal: {solver.is_optimal()}")
```

---

## Solution Class

| Property | Description |
|----------|-------------|
| `cost` | Total cost |
| `num_tours` | Number of routes |
| `is_feasible` | All clients served |
| `unserved` | Unserved client IDs |

### Getting Routes

```python
for tour in solution.get_tours():
    print(f"Route: {tour.get_client_ids()}")
    print(f"Cost: {tour.cost}")
```

---

## Loading Files

```python
# Solomon format (CVRPTW)
problem = routing.load_solomon("c101.txt")

# TSPLIB format (CVRP)  
problem = routing.load_tsplib("A-n32-k5.vrp")
```

---

## Discovery Functions

```python
routing.list_solvers()      # Available solvers
routing.list_attributes()   # Available attributes
routing.get_problem_types() # Problem type requirements
```

---

## Examples

### CVRP

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
```

### CVRPTW

```python
from routing.constants import Attribute

problem = routing.Problem()

# Depot with time window
depot = problem.add_depot(0)
depot.add_attribute(Attribute.GEONODE, 0, 0)
depot.add_attribute(Attribute.RENDEZVOUS, 0, 1000)

# Client with time window
client = problem.add_client(1)
client.add_attribute(Attribute.GEONODE, 20, 20)
client.add_attribute(Attribute.CONSUMER, 10)
client.add_attribute(Attribute.RENDEZVOUS, 0, 100)
client.add_attribute(Attribute.SERVICE_QUERY, 10)

# Vehicle
vehicle = problem.add_vehicle(0)
vehicle.add_attribute(Attribute.STOCK, 50)

solution = routing.solve(problem, "alns", timeout=60)
```

---

## See Also

- [Attributes](attributes.md) - Detailed attribute documentation
- [Solvers](solvers.md) - Solver comparison and tuning
