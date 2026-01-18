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
depot.add_attribute(Attribute.GEONODE, x=0, y=0)

# Add client
client = problem.add_client(1)
client.add_attribute(Attribute.GEONODE, x=10, y=20)
client.add_attribute(Attribute.CONSUMER, demand=15)

# Add vehicle
vehicle = problem.add_vehicle(0)
vehicle.add_attribute(Attribute.STOCK, capacity=100)

# Solve (attributes auto-enabled!)
solution = routing.solve(problem, "ga", timeout=30)
print(f"Cost: {solution.cost}")
```

---

## Attributes

Add attributes using the `Attribute` enum from `routing.constants`:

```python
from routing.constants import Attribute

client.add_attribute(Attribute.GEONODE, x=10, y=20)
client.add_attribute(Attribute.CONSUMER, demand=5)
client.add_attribute(Attribute.RENDEZVOUS, open=0, close=100)
client.add_attribute(Attribute.SERVICE_QUERY, service_time=10)
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

# Add attribute (named args recommended)
entity.add_attribute(Attribute.GEONODE, x=10, y=20)

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
depot.add_attribute(Attribute.GEONODE, x=0, y=0)

# Clients
for i in range(1, 11):
    client = problem.add_client(i)
    client.add_attribute(Attribute.GEONODE, x=i*10, y=i*10)
    client.add_attribute(Attribute.CONSUMER, demand=5+i)

# Vehicles
for v in range(3):
    vehicle = problem.add_vehicle(v)
    vehicle.add_attribute(Attribute.STOCK, capacity=50)

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
depot.add_attribute(Attribute.GEONODE, x=0, y=0)
depot.add_attribute(Attribute.RENDEZVOUS, open=0, close=1000)

# Client with time window
client = problem.add_client(1)
client.add_attribute(Attribute.GEONODE, x=20, y=20)
client.add_attribute(Attribute.CONSUMER, demand=10)
client.add_attribute(Attribute.RENDEZVOUS, open=0, close=100)
client.add_attribute(Attribute.SERVICE_QUERY, service_time=10)

# Vehicle
vehicle = problem.add_vehicle(0)
vehicle.add_attribute(Attribute.STOCK, capacity=50)

solution = routing.solve(problem, "alns", timeout=60)
```

---

## Visualization

The library includes built-in visualization support using matplotlib.

:::{note}
Requires matplotlib: `pip install matplotlib`
:::

### Plot Solution

```python
from routing.visualization import plot_solution

# Basic plot
fig, ax = plot_solution(problem, solution)

# Customize
plot_solution(problem, solution,
              show_demand=True,
              show_arrows=True,
              title="My CVRP Solution",
              save_path="solution.png")
```

### Plot Problem

```python
from routing.visualization import plot_problem

# Show client locations and demands
fig, ax = plot_problem(problem, show_demand=True)
```

### Plot Convergence

```python
from routing.visualization import plot_convergence

# Collect costs during solving
costs = []
def callback(solution, cost):
    costs.append(cost)

solution = routing.solve_with_callback(problem, callback, "ga", 30)

# Plot convergence curve
plot_convergence(costs, title="GA Convergence")
```

### Compare Solvers

```python
from routing.visualization import compare_solutions

solutions = {
    "GA": routing.solve(problem, "ga", timeout=30),
    "ALNS": routing.solve(problem, "alns", timeout=30),
    "VNS": routing.solve(problem, "vns", timeout=30),
}

# Side-by-side comparison
compare_solutions(problem, solutions)
```

### Gantt Chart (Time Windows)

For VRPTW problems, visualize the schedule with time windows:

```python
from routing.visualization import plot_gantt

# Plot schedule showing time windows and service times
fig, ax = plot_gantt(problem, solution)

# Customize
plot_gantt(problem, solution,
           show_travel=True,     # Show travel time bars
           show_tw_bounds=True,  # Show time window boundaries
           speed=1.0,            # Travel speed for time calculation
           title="Vehicle Schedules")
```

The Gantt chart shows:
- **Gray bars**: Time window constraints [open, close]
- **Colored bars**: Service time at each client
- **Thin bars**: Travel time between clients (optional)

### Visualization Options

| Parameter | Default | Description |
|-----------|---------|-------------|
| `show_labels` | `True` | Show client ID labels |
| `show_demand` | `False` | Show demand values |
| `show_arrows` | `True` | Show direction arrows |
| `show_legend` | `True` | Show route legend |
| `route_width` | `1.5` | Line width for routes |
| `client_size` | `100` | Marker size for clients |
| `depot_size` | `200` | Marker size for depot |
| `figsize` | `(10, 8)` | Figure size |
| `save_path` | `None` | Path to save figure |

---

## See Also

- [Attributes](attributes.md) - Detailed attribute documentation
- [Solvers](solvers.md) - Solver comparison and tuning
- [Visualization Notebook](https://github.com/sohaibafifi/rihla/blob/develop/python/examples/notebooks/visualization.ipynb) - Interactive examples
