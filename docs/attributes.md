# Attributes

Attributes define problem features and automatically enable matching constraint generators and evaluators.

## Available Attributes

| Attribute | Entity | Description |
|-----------|--------|-------------|
| **GeoNode** | Node | x, y coordinates for distance calculation |
| **Consumer** | Client | Demand at a client |
| **Stock** | Vehicle | Vehicle capacity |
| **Rendezvous** | Node | Time window bounds (earliest, latest) |
| **ServiceQuery** | Client | Service time at a node |
| **Profiter** | Client | Profit value for orienteering problems (TOP) |
| **Pickup** | Client | Pickup demand for pickup & delivery |
| **Delivery** | Client | Delivery demand for pickup & delivery |
| **Synced** | Client | Temporal synchronization constraints |
| **SoftTimeWindows** | Client | Penalty-based soft time windows |

---

## Problem Variants by Attributes

| Problem | Required Attributes |
|---------|---------------------|
| **TSP** | `GeoNode` |
| **VRP** | `GeoNode` |
| **CVRP** | `GeoNode`, `Consumer`, `Stock` |
| **VRPTW** | `GeoNode`, `Rendezvous`, `ServiceQuery` |
| **CVRPTW** | `GeoNode`, `Consumer`, `Stock`, `Rendezvous`, `ServiceQuery` |
| **TOP** | `GeoNode`, `Profiter`, `Rendezvous` |
| **PDVRP** | `GeoNode`, `Pickup`, `Delivery`, `Stock` |
| **VRPTWTD** | All above + `Synced` |

---

## Usage

### Python - Low-Level API

Python uses helper functions to set attributes on entities:

```python
import routing

routing.init()

# Create problem
problem = routing.Problem()

# Add depot and set its attributes
depot = problem.add_depot(0)
routing.set_depot_location(depot, 0.0, 0.0)           # GeoNode
routing.set_depot_time_window(depot, 0.0, 1000.0)     # Rendezvous

# Add client and set its attributes
client = problem.add_client(1)
routing.set_client_location(client, 10.0, 20.0)       # GeoNode
routing.set_client_demand(client, 5)                   # Consumer
routing.set_client_time_window(client, 0.0, 200.0)    # Rendezvous
routing.set_client_service_time(client, 10.0)         # ServiceQuery

# Add vehicle and set its attributes
vehicle = problem.add_vehicle(0)
routing.set_vehicle_capacity(vehicle, 100)             # Stock

# Enable the attributes used
problem.enable_attributes(["GeoNode", "Consumer", "Stock", "Rendezvous", "ServiceQuery"])

# Solve
solver = routing.create_solver("ga", problem)
solver.solve(30.0)
```

**Python helper functions → Attributes:**

| Helper Function | Attribute Set |
|-----------------|---------------|
| `set_client_location(client, x, y)` | `GeoNode` |
| `set_client_demand(client, demand)` | `Consumer` |
| `set_client_time_window(client, open, close)` | `Rendezvous` |
| `set_client_service_time(client, time)` | `ServiceQuery` |
| `set_vehicle_capacity(vehicle, capacity)` | `Stock` |
| `set_depot_location(depot, x, y)` | `GeoNode` |
| `set_depot_time_window(depot, open, close)` | `Rendezvous` |

### Python - ProblemBuilder (High-Level)

The `ProblemBuilder` wraps the low-level API for convenience:

```python
from routing.problem import ProblemBuilder

problem = (ProblemBuilder()
    .with_depot(0, 0, tw_open=0, tw_close=1000)
    .add_client(1, x=10, y=20, demand=5, tw_open=0, tw_close=200, service_time=10)
    .add_vehicles(count=2, capacity=20)
    .build())  # Automatically calls enable_attributes()
```

### C++

In C++, you use template methods to add attributes:

```cpp
// 1. Enable the attributes you need
problem.enableAttributes<GeoNode, Consumer, Stock, Rendezvous, ServiceQuery>();

// 2. Add depot with attributes
auto* depot = problem.addDepot(0);
depot->addAttribute<GeoNode>(0.0, 0.0);           // Coordinates
depot->addAttribute<Rendezvous>(0.0, 1000.0);     // Operating hours

// 3. Add client with attributes
auto* client = problem.addClient(1);
client->addAttribute<GeoNode>(10.0, 20.0);        // Coordinates
client->addAttribute<Consumer>(5);                 // Demand
client->addAttribute<Rendezvous>(0.0, 200.0);     // Time window
client->addAttribute<ServiceQuery>(10.0);          // Service time

// 4. Add vehicle with attributes
auto* vehicle = problem.addVehicle(0);
vehicle->addAttribute<Stock>(20);                  // Capacity
```

---

## How Attributes Work

When you add an attribute to an entity, the system automatically:

1. **Registers the attribute** with the problem
2. **Enables constraint generators** that depend on that attribute
3. **Enables evaluators** for solution quality assessment

```
Attribute Added → Constraint Generator Activated → Constraints in Model
     ↓
GeoNode        → RoutingGenerator              → Distance objective
Consumer       → CapacityGenerator             → Load ≤ Capacity
Stock          → CapacityGenerator             → Vehicle capacity limit
Rendezvous     → TimeWindowGenerator           → Arrival ∈ [open, close]
ServiceQuery   → TimeWindowGenerator           → Service time in scheduling
```

---

## Custom Attributes

You can define custom attributes using the CRTP pattern:

```cpp
struct MyAttribute : public Attribute<MyAttribute> {
    static constexpr const char* name() { return "MyAttribute"; }

    double value;

    explicit MyAttribute(double v) : value(v) {}
};
```

See [Composable System](composable.md) for more details on extending the attribute system.
